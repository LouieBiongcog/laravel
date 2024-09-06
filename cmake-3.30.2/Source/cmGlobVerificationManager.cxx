/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmGlobVerificationManager.h"

#include <sstream>

#include "cmsys/FStream.hxx"

#include "cmGeneratedFileStream.h"
#include "cmGlobCacheEntry.h"
#include "cmListFileCache.h"
#include "cmMessageType.h"
#include "cmMessenger.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmVersion.h"

bool cmGlobVerificationManager::SaveVerificationScript(const std::string& path,
                                                       cmMessenger* messenger)
{
  if (this->Cache.empty()) {
    return true;
  }

  std::string scriptFile = cmStrCat(path, "/CMakeFiles");
  std::string stampFile = scriptFile;
  cmSystemTools::MakeDirectory(scriptFile);
  scriptFile += "/VerifyGlobs.cmake";
  stampFile += "/cmake.verify_globs";
  cmGeneratedFileStream verifyScriptFile(scriptFile);
  verifyScriptFile.SetCopyIfDifferent(true);
  if (!verifyScriptFile) {
    cmSystemTools::Error("Unable to open verification script file for save. " +
                         scriptFile);
    cmSystemTools::ReportLastSystemError("");
    return false;
  }

  verifyScriptFile << std::boolalpha;
  verifyScriptFile << "# CMAKE generated file: DO NOT EDIT!\n"
                   << "# Generated by CMake Version "
                   << cmVersion::GetMajorVersion() << "."
                   << cmVersion::GetMinorVersion() << "\n";

  verifyScriptFile << "cmake_policy(SET CMP0009 NEW)\n";

  for (auto const& i : this->Cache) {
    CacheEntryKey k = std::get<0>(i);
    CacheEntryValue v = std::get<1>(i);

    if (!v.Initialized) {
      continue;
    }

    verifyScriptFile << "\n";

    for (auto const& bt : v.Backtraces) {
      verifyScriptFile << "# " << std::get<0>(bt);
      messenger->PrintBacktraceTitle(verifyScriptFile, std::get<1>(bt));
      verifyScriptFile << "\n";
    }

    k.PrintGlobCommand(verifyScriptFile, "NEW_GLOB");
    verifyScriptFile << "\n";

    verifyScriptFile << "set(OLD_GLOB\n";
    for (const std::string& file : v.Files) {
      verifyScriptFile << "  \"" << file << "\"\n";
    }
    verifyScriptFile << "  )\n";

    verifyScriptFile << "if(NOT \"${NEW_GLOB}\" STREQUAL \"${OLD_GLOB}\")\n"
                     << "  message(\"-- GLOB mismatch!\")\n"
                     << "  file(TOUCH_NOCREATE \"" << stampFile << "\")\n"
                     << "endif()\n";
  }
  verifyScriptFile.Close();

  cmsys::ofstream verifyStampFile(stampFile.c_str());
  if (!verifyStampFile) {
    cmSystemTools::Error("Unable to open verification stamp file for write. " +
                         stampFile);
    return false;
  }
  verifyStampFile << "# This file is generated by CMake for checking of the "
                     "VerifyGlobs.cmake file\n";
  this->VerifyScript = scriptFile;
  this->VerifyStamp = stampFile;
  return true;
}

bool cmGlobVerificationManager::DoWriteVerifyTarget() const
{
  return !this->VerifyScript.empty() && !this->VerifyStamp.empty();
}

bool cmGlobVerificationManager::CacheEntryKey::operator<(
  const CacheEntryKey& r) const
{
  if (this->Recurse < r.Recurse) {
    return true;
  }
  if (this->Recurse > r.Recurse) {
    return false;
  }
  if (this->ListDirectories < r.ListDirectories) {
    return true;
  }
  if (this->ListDirectories > r.ListDirectories) {
    return false;
  }
  if (this->FollowSymlinks < r.FollowSymlinks) {
    return true;
  }
  if (this->FollowSymlinks > r.FollowSymlinks) {
    return false;
  }
  if (this->Relative < r.Relative) {
    return true;
  }
  if (this->Relative > r.Relative) {
    return false;
  }
  if (this->Expression < r.Expression) {
    return true;
  }
  if (this->Expression > r.Expression) {
    return false;
  }
  return false;
}

void cmGlobVerificationManager::CacheEntryKey::PrintGlobCommand(
  std::ostream& out, const std::string& cmdVar)
{
  out << "file(GLOB" << (this->Recurse ? "_RECURSE " : " ");
  out << cmdVar << " ";
  if (this->Recurse && this->FollowSymlinks) {
    out << "FOLLOW_SYMLINKS ";
  }
  out << "LIST_DIRECTORIES " << this->ListDirectories << " ";
  if (!this->Relative.empty()) {
    out << "RELATIVE \"" << this->Relative << "\" ";
  }
  out << "\"" << this->Expression << "\")";
}

void cmGlobVerificationManager::AddCacheEntry(
  const cmGlobCacheEntry& entry, const std::string& variable,
  const cmListFileBacktrace& backtrace, cmMessenger* messenger)
{
  CacheEntryKey key =
    CacheEntryKey(entry.Recurse, entry.ListDirectories, entry.FollowSymlinks,
                  entry.Relative, entry.Expression);
  CacheEntryValue& value = this->Cache[key];
  if (!value.Initialized) {
    value.Files = entry.Files;
    value.Initialized = true;
    value.Backtraces.emplace_back(variable, backtrace);
  } else if (value.Initialized && value.Files != entry.Files) {
    std::ostringstream message;
    message << std::boolalpha;
    message << "The glob expression\n ";
    key.PrintGlobCommand(message, variable);
    message << "\nwas already present in the glob cache but the directory "
               "contents have changed during the configuration run.\n";
    message << "Matching glob expressions:";
    for (auto const& bt : value.Backtraces) {
      message << "\n  " << std::get<0>(bt);
      messenger->PrintBacktraceTitle(message, std::get<1>(bt));
    }
    messenger->IssueMessage(MessageType::FATAL_ERROR, message.str(),
                            backtrace);
  } else {
    value.Backtraces.emplace_back(variable, backtrace);
  }
}

std::vector<cmGlobCacheEntry> cmGlobVerificationManager::GetCacheEntries()
  const
{
  std::vector<cmGlobCacheEntry> entries;
  for (auto const& i : this->Cache) {
    CacheEntryKey k = std::get<0>(i);
    CacheEntryValue v = std::get<1>(i);
    if (v.Initialized) {
      entries.emplace_back(k.Recurse, k.ListDirectories, k.FollowSymlinks,
                           k.Relative, k.Expression, v.Files);
    }
  }
  return entries;
}

void cmGlobVerificationManager::Reset()
{
  this->Cache.clear();
  this->VerifyScript.clear();
  this->VerifyStamp.clear();
}