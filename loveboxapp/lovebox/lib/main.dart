import 'package:flutter/material.dart';
import 'package:lovebox/firebase_realtime_demo.dart';
import 'package:lovebox/splash_screen.dart';

void main() {
  runApp(MyApp());
}

class MyApp extends StatelessWidget {
  // This widget is the root of your application.
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      debugShowCheckedModeBanner: false,

      theme: ThemeData.dark(),
      home: Splash(),
    );
  }
}
