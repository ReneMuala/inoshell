enum NodeType {
  FILE_NODE,
  DIR_NODE,
  NONE_NODE
};

class Node {
public:
  NodeType type = NONE_NODE;
  int dir = -1;
  String name;
  String content;

  int getSize(){
    return name.length() + content.length();
  }
};

namespace FileSystem {
  Node nodes[20];

  int cf = 0;

  String getPath(){
    String path;
    Node * curr = &nodes[cf];
    do {
      path = curr->name + path;
      if(curr->dir != -1){
        curr = &nodes[curr ->dir]; 
        continue;
      }
    } while(curr->dir != -1);
    return path;
  }

  bool exists(String name){
    for(Node & node : nodes){
      if(node.dir == cf && node.type != NONE_NODE && (node.name == name || node.name == name + "/")){
        Serial.println("File/dir " + name + " already exists");
        return true;
      }
    }
    return false;
  }

  void write(String filename, String content){
    for(Node & node : nodes){
      if(node.dir == cf && node.type == FILE_NODE && node.name == filename){
        node.content = content;
        return;
      }
    }
    Serial.println("file '"+filename+"' not found");
  }

  void touch(String filename){
    if(exists(filename)){
      return;
    }
    for(Node & node : nodes){
      if(node.type == NONE_NODE){
        node.name = filename;
        node.type = FILE_NODE;
        node.dir = cf;
        node.content = "";
        return;
      }
    }
    Serial.println("No space left in device");
  }

  void makeDir(String name){
    if(exists(name)){
      return;
    }

    for(Node & node : nodes){
      if(node.type == NONE_NODE){
        node.name = name;
        node.type = DIR_NODE;
        node.dir = cf;
        node.content = "";
        if(!node.name.endsWith("/")){
          node.name += "/";
        }
        return;
      }
    }
    Serial.println("No space left in device");
  }

  void cd(String name){
    if(name == ".."){
      Node * cur = &nodes[cf];
      for(int i = 0 ; i < 20; i++){
        if(i == cur->dir){
          cf = i;
        }
      }
    } else if(name == ".") {

    } else {
      for(int i = 0 ; i < 20; i++){
        Node & node = nodes[i];
        if(!name.endsWith("/")){
          name+="/";
        }

        if(node.dir == FileSystem::cf && node.name == name && node.type == DIR_NODE){
          cf = i;
        }
      }
    }
  }

  void rm(String name){
    for(Node & node :  FileSystem::nodes){
      if(node.dir == FileSystem::cf && node.name == name){
        node.type = NONE_NODE;
        return;
      }
    }
    Serial.println(name + " not found");
  }

  void cat(String name){
    for(Node & node :  FileSystem::nodes){
      if(node.dir == FileSystem::cf && node.name == name && node.type == FILE_NODE){
        Serial.print(node.content);
        break;
      }
    }
  }

  void init(){
    Node * node = &nodes[cf];
    if(node->type == NONE_NODE){
      node->type = DIR_NODE;
      node->name = "/";
    }
  }
}


enum ShellState {
  READ,
  WRITE
};

class Shell {
  public:

  ShellState state = WRITE;

  void cmd_clear(){
    for(int i = 0 ; i < 20; i++)
      Serial.println();
  }

  void cmd_mkdir(String cmd){
    if(cmd.length() > 5){
      FileSystem::makeDir(cmd.substring(6));
    }
  }

  void cmd_ls(){
    for(Node & node :  FileSystem::nodes){
      if(node.dir == FileSystem::cf && node.type != NONE_NODE){
        Serial.println(node.name);
      }
    }
  }
  
  void cmd_echo(String cmd){
    if(cmd.length() > 4){
      Serial.println(cmd.substring(5));
    } else {
      Serial.println();
    }
  }

  void cmd_cd(String cmd){
    if(cmd.length() > 2){
      FileSystem::cd(cmd.substring(3));
    } else {
      Serial.println("Specify dir");
    }
  }

  
  void cmd_rm(String cmd){
    if(cmd.length() > 2){
      FileSystem::rm(cmd.substring(3));
    } else {
      Serial.println("Specify dir");
    }
  }

  void cmd_touch(String cmd){
    if(cmd.length() > 5){
      FileSystem::touch(cmd.substring(6));
    } else {
      Serial.println("Specify file");
    }
  }

  String get_arg(String cmd, int offset, bool ignore_space = false){
    String arg;
    for(int i = offset; i < cmd.length() && (ignore_space || cmd.c_str()[i] != ' '); i++){
      arg += cmd.c_str()[i];
    }
    return arg;
  }


  void cmd_write(String cmd){
    String filename = get_arg(cmd, 6);
    String content = get_arg(cmd, 7 + filename.length(), true);
    FileSystem::write(filename, content);
  }

  void cmd_cat(String cmd){
    if(cmd.length() > 3){
      FileSystem::cat(cmd.substring(4));
    }
  }

  bool read(){
    if(Serial.available()){
      String cmd = Serial.readString();
      cmd.remove(cmd.length() - 1);
      Serial.println(cmd);

      if(cmd.startsWith("clear")){
        cmd_clear();
      } else if(cmd.startsWith("echo")){
        cmd_echo(cmd);  
      } else if(cmd.startsWith("mkdir")){
        cmd_mkdir(cmd);  
      } else if(cmd.startsWith("ls")){
        cmd_ls();  
      } else if(cmd.startsWith("cd")){
        cmd_cd(cmd);  
      }  else if(cmd.startsWith("rm")){
        cmd_rm(cmd);  
      } else if(cmd.startsWith("touch")){
        cmd_touch(cmd);  
      } else if(cmd.startsWith("write")){
        cmd_write(cmd);  
      }  else if(cmd.startsWith("cat")){
        cmd_cat(cmd);  
      }  else {
        Serial.println("Unknown command: " + cmd);
      }
      return true;
    }
    return false;
  }

  void write(){
    String path = FileSystem::getPath();
    Serial.print("ino:" + path + "$ ");
  }

  void handle(){
    if(state == READ){
      if(read()){
        state = WRITE;
      }
    } else {
      write();
      state = READ;
    }
  }

};

Shell shell;

void setup(){
  FileSystem::init();
  Serial.begin(9600);
}

void loop(){
  shell.handle();
  delay(50);
}
