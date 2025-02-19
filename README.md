# Compiler Implementation Project

This project is a compiler for a simple programming language, developed as part of a class project. The compiler parses input programs, generates an intermediate representation in the form of a linked list of instructions, and executes the program by interpreting the intermediate representation. 

## **Project Description**
The compiler supports the following language constructs:
- **Assignment statements**
- **Input/output statements**
- **Control structures**: `if`, `while`, `for`, and `switch` statements

The intermediate representation allows execution by traversing the instruction list, evaluating conditions, and performing computations based on a virtual memory representation.

### **Key Features**
1. **Lexical Analysis**: Tokenizes the input source code.
2. **Syntax Parsing**: Validates the input program against the given grammar and constructs an abstract syntax tree (AST).
3. **Intermediate Representation**: Generates a graph-like data structure representing the program for execution.
4. **Execution Engine**: Executes the program instructions sequentially or conditionally (e.g., via jumps).

### **How It Works**
1. **Parsing**: Input programs are parsed using a custom lexer and parser.
2. **Intermediate Representation**: A linked list of instruction nodes is generated, where each node represents an operation like assignment, conditional jump, or input/output.
3. **Execution**: Instructions are executed based on the program counter, simulating memory operations and control flow.

### **Language Grammar**
The grammar defines the syntax of the input language and supports:
- Variables and assignments
- Arithmetic operations (`+`, `-`, `*`, `/`)
- Control structures (`if`, `while`, `for`, `switch`)
- Input and output statements

**P.S. This is a school project and I do not consent to use this project to anyone!!
