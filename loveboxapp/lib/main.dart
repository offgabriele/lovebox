import 'package:firebase_core/firebase_core.dart';
import 'package:firebase_database/firebase_database.dart';
import 'package:firebase_database/ui/firebase_animated_list.dart';
import 'package:firebase_database/ui/firebase_list.dart';
import 'package:flutter/material.dart';

void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  await Firebase.initializeApp();
  runApp(MyApp());
}

class MyApp extends StatelessWidget {
  // This widget is the root of your application.
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      debugShowCheckedModeBanner: false,
      title: 'Pets',
      theme: ThemeData(
        primarySwatch: Colors.blue,
      ),
      home: Home(title: "Message"),
    );
  }
}

class TextInputWidget extends StatefulWidget {
  final Function(String) callback;

  TextInputWidget(this.callback);

  @override
  _TextInputWidgetState createState() => _TextInputWidgetState();
}

class _TextInputWidgetState extends State<TextInputWidget> {
  final controller = TextEditingController();

  @override
  void dispose() {
    super.dispose();
    controller.dispose();
  }

  void click() {
    widget.callback(controller.text);
    FocusScope.of(context).unfocus();
    controller.clear();
  }

  @override
  Widget build(BuildContext context) {
    return TextField(
        controller: this.controller,
        decoration: InputDecoration(
            prefixIcon: Icon(Icons.message),
            labelText: "Type a message:",
            suffixIcon: IconButton(
              icon: Icon(Icons.send),
              splashColor: Colors.blue,
              tooltip: "Send love",
              onPressed: this.click,
            )));
  }
}


class Home extends StatefulWidget {
  Home({Key key, this.title}) : super(key: key);
  final String title;

  @override
  _HomeState createState() => _HomeState();
}

class _HomeState extends State<Home> {
  final dbRef = FirebaseDatabase.instance.reference().child("messages").orderByChild('timestamp');
  List<Map<dynamic, dynamic>> lists = [];

  Future<DatabaseReference> sendMessage(String text) async {
    Map<String, dynamic> latest = (await FirebaseDatabase.instance.reference().child("messages/latest").once()).value;
    print(latest);
    String newchild = latest["timesent"].toString().replaceAll("/", " ");
    await FirebaseDatabase.instance.reference().child("messages/" + newchild).set(latest);
    var currDt = DateTime.now();
    Map<String, dynamic> newmessage = {
    'message' : text,
    'type' : 'message',
    'timesent': "${currDt.year.toString()}/${currDt.month.toString()}/${currDt.day.toString()} ${currDt.hour.toString()}:${currDt.minute.toString()}:${currDt.second.toString()}",
    "timestamp" : currDt.millisecondsSinceEpoch,
    'displayed' : "false",
    'read' : "false"
    };
     await FirebaseDatabase.instance.reference().child("messages/latest").set(newmessage);
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
        appBar: AppBar(
          title: Text(widget.title),
        ),
        body: Column(
          children: [
            Expanded(
                          child: FirebaseAnimatedList(query: dbRef, itemBuilder: (context, snapshot, animation, index) {
                return Card(
                  child: Row(children: <Widget>[
                Expanded(
                    child: ListTile(
                  title: Text(snapshot.value["message"]),
                  // subtitle: Text(post.author),
                )),
                
              ]));
              },),
            ),
            TextInputWidget(this.sendMessage)
          ],
        ));
  }
}