with Ada.Streams.Stream_IO,
     Message,
     Ada.Text_IO,
     Protocol_Buffers.Message;

procedure Main is
begin
   declare
      Person        : Message.Person.Instance;
      Output_Stream : Ada.Streams.Stream_IO.Stream_Access;
      Some_File     : Ada.Streams.Stream_IO.File_Type;
   begin
      Message.Person.Set_Age (The_Message => Person,
                              Value       => 4);
      Message.Person.Set_Id  (The_Message => Person,
                              Value       => 2);
      Ada.Text_IO.Put_Line ("Instance to save to file:");
      Ada.Text_IO.Put_Line ("Age: " & Message.Person.Age (The_Message => Person)'Img);
      Ada.Text_IO.Put_Line ("Id:  " & Message.Person.Id (The_Message => Person)'Img);

      Ada.Streams.Stream_IO.Create (File => Some_File,
                                    Name => "asdfk.dat");

      Output_Stream := Ada.Streams.Stream_IO.Stream (Some_File);

      Protocol_Buffers.Message.Serialize_Partial_To_Output_Stream (The_Message   => Person,
                                                                   Output_Stream => Output_Stream);

      Ada.Streams.Stream_IO.Close (Some_File);
   end;

   declare
      Person2 : Message.Person.Instance;
      Input_Stream : Ada.Streams.Stream_IO.Stream_Access;
      Input_File   : Ada.Streams.Stream_IO.File_Type;
   begin
      Ada.Streams.Stream_IO.Open (File => Input_File,
                                  Mode => Ada.Streams.Stream_IO.In_File,
                                  Name => "asdfk.dat");

      Input_Stream := Ada.Streams.Stream_IO.Stream (Input_File);

      Protocol_Buffers.Message.Parse_From_Input_Stream (The_Message  => Person2,
                                                        Input_Stream => Input_Stream);

      Ada.Text_IO.Put_Line ("Instance to read from file:");
      Ada.Text_IO.Put_Line ("Age: " & Message.Person.Age (The_Message => Person2)'Img);
      Ada.Text_IO.Put_Line ("Id:  " & Message.Person.Id (The_Message => Person2)'Img);

      Ada.Streams.Stream_IO.Close (Input_File);
   end;

end Main;
