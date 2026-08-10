# Ownership
This file specifies what struct owns which dynamically allocated data, and what other struct references it.


```mermaid
classDiagram {
    class Token {
        ...
        char* value
    }
    class Node {}
    class NodeList {
        ...
        char* func_def.ident,
        char* param.ident,
        char* var.ident,
        char* call.ident,
        char* literal.value,
        char* builtin.ident
    }
    class Symbol {
        char* ident
    }
    class BuiltinDef {
        
    }
    class Scope {}
    class SymTable {}
    class IRFunc {}
    class IRInstr {}
    class IROperand {}
    class StackFrame {}
    class SFEntry {}
}
```