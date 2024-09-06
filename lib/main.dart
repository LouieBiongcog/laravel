import 'package:flutter/material.dart';

void main(){
  runApp(const MyApp());
}

class MyApp extends StatelessWidget{
  const MyApp ({super.key});

  @override
  Widget build(BuildContext context){
    return MaterialApp(
      title: 'Flutter Demo',
      theme: ThemeData(
        primarySwatch: Colors.green,
      ),
      home: const MyHomePage(), 
    );
  }
}

class MyHomePage extends StatelessWidget{
  const MyHomePage({super.key});

  @override
  Widget build(BuildContext context){
    return Scaffold(
      appBar: AppBar(
        backgroundColor: Colors.red,
        title: const Text('Flutter Demo'), 
      ),
      backgroundColor: Colors.pink,
      body: Center(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: <Widget>[
            const MyButton(), 
            const SizedBox(height: 1),
            const Icon(
              Icons.computer,
              color: Colors.green,
              size : 100.0,
            ),
            const SizedBox(height: 1),
            Image(image: AssetImage('assets/pic.png'),
            height: 100.0,width: 100.0,
            ),
            
          ],
        ),
      ),
    );
  }
}

class MyButton extends StatelessWidget{
  const MyButton({super.key}); 

  @override
  Widget build(BuildContext context){
    return ElevatedButton(
      style: ElevatedButton.styleFrom(
        padding: const EdgeInsets.symmetric(horizontal: 30, vertical: 15),
         backgroundColor: Colors.green, 
        textStyle: const TextStyle( 
          fontSize: 20,
          fontWeight: FontWeight.bold,
         
        ),
      ),
      onPressed: (){
        print('Thank you for Clicking');
      },
      child: const Text('Click Me'), 
    );
  }
}
