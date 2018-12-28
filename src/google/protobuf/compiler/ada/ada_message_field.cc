// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
// http://code.google.com/p/protobuf/
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// Author: kenton@google.com (Kenton Varda)
//  Based on original Protocol Buffers design by
//  Sanjay Ghemawat, Jeff Dean, and others.

#include <google/protobuf/compiler/ada/ada_message_field.h>
#include <google/protobuf/io/printer.h>
#include <google/protobuf/compiler/ada/ada_helpers.h>

namespace google {
namespace protobuf {
namespace compiler {
namespace ada {

namespace {

void SetMessageVariables(const FieldDescriptor* descriptor,
  map<string, string>* variables) {
  SetCommonFieldVariables(descriptor, variables);
  (*variables)["containing_type"] = FieldMessageContainingPackageName(descriptor);
  (*variables)["type"] = FieldMessageTypeName(descriptor);
}

} // namespace

MessageFieldGenerator::
MessageFieldGenerator(const FieldDescriptor* descriptor)
: descriptor_(descriptor) {
  SetMessageVariables(descriptor, &variables_);
}

MessageFieldGenerator::
~MessageFieldGenerator() { }

void MessageFieldGenerator::
GenerateAccessorDeclarations(io::Printer* printer) const {
  // Generate declaration Get_$name$
  printer->Print(variables_,
    "function Get_$name$\n");
  printer->Indent();
  printer->Print(variables_,
    "(The_Message : in out $packagename$.Instance) return access $containing_type$.Instance;\n");
  printer->Outdent();

  // Generate declaration Release_$name$
  printer->Print(variables_,
    "function Release_$name$\n");
  printer->Indent();
  printer->Print(variables_,
    "(The_Message : in out $packagename$.Instance) return access $containing_type$.Instance;\n");
  printer->Outdent();

  // Generate declaration Set_$name$
  printer->Print(variables_,
    "procedure Set_$name$\n");
  printer->Indent();
  printer->Print(variables_,
    "(The_Message : in out $packagename$.Instance;\n"
    " Value : in $containing_type$.$type$_Access);\n");
  printer->Outdent();
}

void MessageFieldGenerator::
GenerateAccessorDefinitions(io::Printer* printer) const {
  // Generate body Get_$name$
  printer->Print(variables_,
    "function Get_$name$\n");
  printer->Indent();
  printer->Print(variables_,
    "(The_Message : in out $packagename$.Instance) return access $containing_type$.Instance is\n"
    "use type $containing_type$.$type$_Access;\n");
  printer->Outdent();
  printer->Print(
    "begin\n");
  printer->Indent();
  printer->Print(variables_,
    "The_Message.Set_Has_$name$;\n"
    "if The_Message.$name$ = null then\n");
  printer->Indent();
  printer->Print(variables_,
    "The_Message.$name$ := $containing_type$.$type$_Access'(new $containing_type$.Instance);\n");
  printer->Outdent();
  printer->Print(variables_,
    "end if;\n"
    "return The_Message.$name$;\n");
  printer->Outdent();
  printer->Print(variables_,
    "end Get_$name$;\n"
    "\n");

  // Generate body Release_$name$
  printer->Print(variables_,
    "function Release_$name$\n");
  printer->Indent();
  printer->Print(variables_,
    "(The_Message : in out $packagename$.Instance) return access $containing_type$.Instance is\n"
    "Temp : access $containing_type$.Instance;\n");
  printer->Outdent();
  printer->Print(
    "begin\n");
  printer->Indent();
  printer->Print(variables_,
    "The_Message.Clear_Has_$name$;\n"
    "Temp := The_Message.$name$;\n"
    "The_Message.$name$ := null;\n"
    "return Temp;\n");
  printer->Outdent();
  printer->Print(variables_,
    "end Release_$name$;\n"
    "\n");

  // Generate body Set_$name$
  printer->Print(variables_,
    "procedure Set_$name$\n");
  printer->Indent();
  // TODO: Move declaration of Free outside Set_$name$. Fix use type?
  printer->Print(variables_,
    "(The_Message : in out $packagename$.Instance;\n"
    " Value : in $containing_type$.$type$_Access) is\n"
    "use type $containing_type$.$type$_Access;\n"
    "Temp : Protocol_Buffers.Message.Instance_Access := Protocol_Buffers.Message.Instance_Access (The_Message.$name$);\n");
  printer->Outdent();
  printer->Print(
    "begin\n");
  printer->Indent();
  printer->Print(variables_,
    "Protocol_Buffers.Message.Free (Temp);\n"
    "The_Message.$name$ := Value;\n"
    "if The_Message.$name$ /= null then\n");
  printer->Indent();
  printer->Print(variables_,
    "The_Message.Set_Has_$name$;\n");
  printer->Outdent();
  printer->Print(
    "else\n");
  printer->Indent();
  printer->Print(variables_,
    "The_Message.Clear_Has_$name$;\n");
  printer->Outdent();
  printer->Print(
    "end if;\n");
  printer->Outdent();
  printer->Print(variables_,
    "end Set_$name$;\n");
}

void MessageFieldGenerator::
GenerateClearingCode(io::Printer* printer) const {
  // TODO: Optimize! We _really_ don't want to free a message
  //  but instead reuse it to avoid allocating and deallocating
  //  storage. This would involve quite a bit of extra work ...
  printer->Print(variables_,
    "The_Message.Clear_Has_$name$;\n");
  GenerateFinalizationCode(printer);
}

void MessageFieldGenerator::
GenerateRecordComponentDeclaration(io::Printer* printer) const {
  printer->Print(variables_,
    "$name$ : access $containing_type$.Instance;\n");
}

void MessageFieldGenerator::
GenerateSerializeWithCachedSizes(io::Printer* printer) const {
  printer->Print(variables_,
    "The_Coded_Output_Stream.Write_Message ($number$, The_Message.$name$.all);\n");
}

void MessageFieldGenerator::
GenerateByteSize(io::Printer* printer) const {
  printer->Print(variables_,
    "Total_Size := Total_Size + $tag_size$ + "
    "Protocol_Buffers.IO.Coded_Output_Stream.Compute_Message_Size_No_Tag ("
    "The_Message.$name$.all);\n");
}

void MessageFieldGenerator::
GenerateMergeFromCodedInputStream(io::Printer* printer) const {
  if (descriptor_->type() == FieldDescriptor::TYPE_MESSAGE) {
    printer->Print(variables_,
      "The_Coded_Input_Stream.Read_Message (The_Message.Get_$name$.all);\n");
  } else {
    // TODO: implement handling of groups?
  }
}

void MessageFieldGenerator::
GenerateMergingCode(io::Printer* printer) const {
  printer->Print(variables_,
    "To.Get_$name$.Merge (From.$name$.all);\n");
}

void MessageFieldGenerator::
GenerateStaticDefaults(io::Printer* printer) const { }

void MessageFieldGenerator::
GenerateFinalizationCode(io::Printer* printer) const {
  // TODO: Consider changing this to a procedure.
  printer->Print(
    "declare\n");
  printer->Indent();
  // Move this elsewhere
  printer->Print(variables_,
    "Temp : Protocol_Buffers.Message.Instance_Access := "
    "Protocol_Buffers.Message.Instance_Access(The_Message.$name$);\n");
  printer->Outdent();
  printer->Print(
    "begin\n");
  printer->Indent();
  printer->Print(variables_,
    "Protocol_Buffers.Message.Free (Temp);\n"
    "The_Message.$name$ := null;\n");
  printer->Outdent();
  printer->Print(
    "end;\n");
}

// ===================================================================

RepeatedMessageFieldGenerator::
RepeatedMessageFieldGenerator(const FieldDescriptor* descriptor)
: descriptor_(descriptor) {
  SetMessageVariables(descriptor, &variables_);
}

RepeatedMessageFieldGenerator::
~RepeatedMessageFieldGenerator() { }

void RepeatedMessageFieldGenerator::
GenerateAccessorDeclarations(io::Printer* printer) const {
  // Generate declaration for Get_$name$
  // TODO: change index type?
  printer->Print(variables_,
    "function Get_$name$\n");
  printer->Indent();
  printer->Print(variables_,
    "(The_Message : in $packagename$.Instance;\n"
    " Index : in Protocol_Buffers.Wire_Format.PB_Object_Size) "
    "return access $containing_type$.Instance;\n");
  printer->Outdent();

  // Generate declaration for Add_$name$
  printer->Print(variables_,
    "function Add_$name$\n");
  printer->Indent();
  printer->Print(variables_,
    "(The_Message : in out $packagename$.Instance) "
    "return access $containing_type$.Instance;\n");
  printer->Outdent();
}

void RepeatedMessageFieldGenerator::
GenerateAccessorDefinitions(io::Printer* printer) const {
  // Generate body for $name$
  // TODO: change index type?
  printer->Print(variables_,
    "function Get_$name$\n");
  printer->Indent();
  printer->Print(variables_,
    "(The_Message : in $packagename$.Instance;\n"
    " Index : in Protocol_Buffers.Wire_Format.PB_Object_Size) "
    "return access $containing_type$.Instance is\n");
  printer->Outdent();
  printer->Print(
    "begin\n");
  printer->Indent();

  // Body
  printer->Print(variables_,
    "return $containing_type$.$type$_Access (The_Message.$name$.Element (Index));\n");

  printer->Outdent();
  printer->Print(variables_,
    "end Get_$name$;\n"
    "\n");

  // Generate body for Add_$name$
  // TODO: change index type?
  printer->Print(variables_,
    "function Add_$name$\n");
  printer->Indent();
  printer->Print(variables_,
    "(The_Message : in out $packagename$.Instance) return access $containing_type$.Instance is\n"
    "Temp : $containing_type$.$type$_Access := new $containing_type$.Instance;\n");
  printer->Outdent();
  printer->Print(
    "begin\n");
  printer->Indent();

  // Body
  printer->Print(variables_,
    "The_Message.$name$.Append (Protocol_Buffers.Message.Instance_Access (Temp));\n"
    "return Temp;\n");

  printer->Outdent();
  printer->Print(variables_,
    "end Add_$name$;\n");
}

void RepeatedMessageFieldGenerator::
GenerateClearingCode(io::Printer* printer) const {
  // TODO: Optimize! We _really_ don't want to free all messages
  //  but instead reuse them to avoid allocating and deallocating
  //  storage. This would involve quite a bit of extra work ...
  GenerateFinalizationCode(printer);
}

void RepeatedMessageFieldGenerator::
GenerateRecordComponentDeclaration(io::Printer* printer) const {
  // TODO: store vector on heap?
  printer->Print(variables_,
    "$name$ : Protocol_Buffers.Message.Message_Vector.Vector;\n");
}

void RepeatedMessageFieldGenerator::
GenerateSerializeWithCachedSizes(io::Printer* printer) const {
  // TODO: rewrite with cursor?
  printer->Print(variables_,
    "for E of The_Message.$name$ loop\n");
  printer->Indent();
  printer->Print(variables_,
    "The_Coded_Output_Stream.Write_Message ($number$, E.all);\n");
  printer->Outdent();
  printer->Print(
    "end loop;\n");
}

void RepeatedMessageFieldGenerator::
GenerateByteSize(io::Printer* printer) const {
  printer->Print(variables_,
    "Total_Size := Total_Size + $tag_size$ * The_Message.$name$_Size;\n"
    "for E of The_Message.$name$ loop\n");
  printer->Indent();
  printer->Print(
    "Total_Size := Total_Size + "
    "Protocol_Buffers.IO.Coded_Output_Stream.Compute_Message_Size_No_Tag (E.all);\n");
  printer->Outdent();
  printer->Print(
    "end loop;\n");
}

void RepeatedMessageFieldGenerator::
GenerateMergeFromCodedInputStream(io::Printer* printer) const {
  // TODO: consider optimizing. At present we only read one field at time.
  //       It might be beneficial to guess that the next read item from the
  //       Coded_Input_Stream is also of this type ...
  printer->Print(
    "declare\n");
  printer->Indent();
  printer->Print(variables_,
    "Temp : $containing_type$.$type$_Access := The_Message.Add_$name$;\n");
  printer->Outdent();
  printer->Print(
    "begin\n");
  printer->Indent();

  printer->Print(variables_,
    "The_Coded_Input_Stream.Read_Message (Temp.all);\n");

  printer->Outdent();
  printer->Print("end;\n");
}

void RepeatedMessageFieldGenerator::
GenerateMergingCode(io::Printer* printer) const {
  // TODO: optimize ...
  printer->Print(
    "declare\n");
  printer->Indent();
  printer->Print(variables_,
    "Temp : $containing_type$.$type$_Access;\n");
  printer->Outdent();
  printer->Print(
    "begin\n");
  printer->Indent();
  printer->Print(variables_,
    "for E of From.$name$ loop\n");
  printer->Indent();
  printer->Print(variables_,
    "Temp := new $containing_type$.Instance;\n"
    "Temp.Merge ($containing_type$.Instance (E.all));\n"
    "To.$name$.Append (Protocol_Buffers.Message.Instance_Access (Temp));\n");
  printer->Outdent();
  printer->Print(
    "end loop;\n");
  printer->Outdent();
  printer->Print(
    "end;\n");
}

void RepeatedMessageFieldGenerator::
GenerateStaticDefaults(io::Printer* printer) const { }

void RepeatedMessageFieldGenerator::
GenerateFinalizationCode(io::Printer* printer) const {
  // TODO: Consider changing this to a procedure.
  
  // For some reason we can't use a container element iterator here ...
  printer->Print(variables_,
    "for C in The_Message.$name$.Iterate loop\n");
  printer->Indent();
  printer->Print(variables_,
    "Protocol_Buffers.Message.Free (The_Message.$name$.Reference (C).Element.all);\n");
  printer->Outdent();
  printer->Print(
    "end loop;\n");
  printer->Print(variables_,
    "The_Message.$name$.Clear;\n"
    "\n");
}

} // namespace ada
} // namespace compiler
} // namespace protobuf
} // namespace google
