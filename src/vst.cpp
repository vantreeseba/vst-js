// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string.h>
#include <fstream>
#include <iostream>
#include <sstream>

#include "include/libplatform/libplatform.h"
#include "include/v8.h"

using namespace v8;


std::string load_file(std::string fileName) {
  std::ifstream ifs(fileName);
  std::string content((std::istreambuf_iterator<char>(ifs)),
                      (std::istreambuf_iterator<char>()));

  return content;
}

Handle<Value> Require(const FunctionCallbackInfo<Value>& args) {
  auto isolate = Isolate::GetCurrent();
  for (int i = 0; i < args.Length(); i++) {
    String::Utf8Value str(args[i]);

    // load_file loads the file with this name into a string,
    // I imagine you can write a function to do this :)
    auto js_file = load_file(*str);

    if (js_file.length() > 0) {
      auto source = String::NewFromUtf8(isolate, js_file.c_str());
      auto script = Script::Compile(source);
      return script->Run();
    }
  }
  return Undefined(isolate);
}


void initV8(char* argv[]) {
  // Initialize V8.
  V8::InitializeICUDefaultLocation(argv[0]);
  V8::InitializeExternalStartupData(argv[0]);
  Platform* platform = platform::CreateDefaultPlatform();
  V8::InitializePlatform(platform);
  V8::Initialize();

}

void shutdownV8() {
  V8::Dispose();
  V8::ShutdownPlatform();
  // delete platform;
  // delete create_params.array_buffer_allocator;
}

void runInIsolateScope(Isolate* isolate, std::string scriptString) {
  Isolate::Scope isolate_scope(isolate);
  HandleScope handle_scope(isolate);
  Local<Context> context = Context::New(isolate);
  Context::Scope context_scope(context);

  // Create a string containing the JavaScript source code.
  auto source =
      String::NewFromUtf8(isolate, scriptString.c_str(), NewStringType::kNormal)
          .ToLocalChecked();

  auto script = Script::Compile(context, source).ToLocalChecked();
  auto result = script->Run(context).ToLocalChecked();


  // Convert the result to an UTF8 string and print it.
  String::Utf8Value utf8(result);
  std::cout << *utf8 << std::endl;
}

int main(int argc, char* argv[]) {
  std::string scriptString;
  if (argc < 2) {
    std::cout << "You must pass a script to start." << std::endl;
    exit(1);
  } else {
    scriptString = load_file(argv[1]);
    std::cout << "FileName:" << argv[1] << std::endl;
    std::cout << "Contents:" << scriptString << std::endl;
  }
  initV8(argv);

  // Create a new Isolate and make it the current one.
  Isolate::CreateParams create_params;
  create_params.array_buffer_allocator =
      ArrayBuffer::Allocator::NewDefaultAllocator();
  Isolate* isolate = Isolate::New(create_params);

  Handle<ObjectTemplate> global = ObjectTemplate::New();
  global->Set("require"


  runInIsolateScope(isolate, scriptString);

  isolate->Dispose();
  shutdownV8();
  return 0;
}


