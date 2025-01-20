# scratch-lang
Experimental toy language to play with lexers, AST, LLVM
test

basic flow:
Open file of text, creates a buffer/array of characters
Run Lexer class to create a series of Tokens from array of characters
  array of char --> array/list of Token

Token class therefore represents one or more characters in a unit, what is technically referred to as a "lexeme" I believe


Run Parser to walk the list of tokens, in order, creating an AST. Once the AST is built, this would be handed over to the Compiler
class to evaluate and build out LLVM based IR, and from that LLVM takes care of optimization steps and building actual X86/64 machine
code

The Compiler class will have logic that deals with mapping "messages" to operate on instances/variables, either on the heap or on the stack.
Messages can be thought of loosley as functions? Same concept as Smalltalk, only in this case we're not as rigorous (for better or worse). The 
idea here is to be a little more pragmatic, not everything is an object. Primitive types like ints, bools, double, etc can "respond" to messages.
It's the Compiler class's job to manage this, to determing if a message is going to soemthing that is really any "object" instance, or 
if it's dealing with a primitive. If it's a primitive then the message would be interpreted essentially the same as a function call in C 
or anything else. If the message is sent to an object the Compiler needs to figure out if the message can be resolved at compiler time,
in which case it
