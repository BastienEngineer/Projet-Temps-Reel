with Ada.Text_IO; use Ada.Text_IO;
with Ada.Real_Time; use Ada.Real_Time;
with System.Multiprocessors; use System.Multiprocessors;

procedure Simple is
   -- Import the C function stack_prefault defined in prefault.c ...
   procedure stack_prefault;
   pragma Import(C,stack_prefault,"stack_prefault");

   -- Import the C function lock_memory defined in lock.c ...
    procedure lock_memory;
    pragma Import(C,lock_memory,"lock_memory");

   -- Import the C function job defined in jobs.c ...
   procedure job(FET_ns : Long_Integer);
   pragma Import(C,job,"job");
   procedure job1(FET_ns : Long_Integer);
   pragma Import(C,job1,"job1");


   -- Define a task type RT_Task with the following parameters:
   -- * Id is the identifier,
   -- * FET is the fixed execution time in milliseconds (ms),
   -- * Prd is the Period in ms,
   -- * Itr is the number of iterations
   -- * Aff is the core affinity,
   -- * Pri is the priority.
task type RT_Task (Id : Integer; FET : Long_Integer; Prd, Itr : Integer; Aff : CPU; Pri : System.Priority) with CPU => Aff, Priority => Pri;

   -- RT_Task is a periodic task type with implicit deadlines ...
   task body RT_Task is
      -- Set the period ...
      Period : constant Time_Span := Milliseconds(Prd);
      -- Set the deadline ...
      Deadline : constant Time_Span := Milliseconds(Prd);
      -- Set the FET in nanoseconds (ns): long integer to be passed to the imported function Job ...
      FET_ns : constant Long_Integer := FET*1000000;
      -- Define the variable Next to save the next release instants of each job ...
      Next : Ada.Real_Time.Time;
   begin
      -- Set the first release time in Next
      Next := Ada.Real_Time.Clock;
      for J in 1 .. Itr loop
         begin
            -- Start the job ...
                        Job(FET_ns);
                        Job1(FET_ns);

            -- Check if the deadline is respected, print a message if not ...
            if clock - Next > Deadline then
                                Put_Line("deadline passee");
                        end if ;

            -- Set the next release time ...
            Next := Next + Period ;

            -- Delay until Next ...
                        delay until Next;

         end;
      end loop;
   end RT_Task;

   -- Declare a named static task Tau_1 (type RT_Task, Id = 1): the parameters FET, Prd, Itr, Aff and Pri are to be adjusted according to the lab instructions ...
   Tau_1 : RT_Task(1,25,50,1000,2,System.Priority'First);
   -- Declare a named static task Tau_2 (type RT_Task, Id = 2): the parameters FET, Prd, Itr, Aff and Pri are to be adjusted according to the lab instructions ...
   Tau_2 : RT_Task(2,25,50,1000,2,System.Priority'First);

begin
   -- Lock the current and future memory allocations ...
   lock_Memory;
   -- Pre-fault the stack ...
   Stack_Prefault;
end Simple;