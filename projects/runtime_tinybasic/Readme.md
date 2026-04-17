# Grammar
Derived from https://en.wikipedia.org/wiki/Tiny_BASIC
```bnf
program ::= statement-list
statement ::= print-stmt | clear-stmt | if-stmt | input-stmt | binding-stmt | exit-stmt
statement-list ::= statement | statement statement-list
print-stmt ::= PRINT expr-list
clear-stmt ::= CLEAR
if-stmt ::= IF expr relop expr THEN statment-list END
while-stmt ::= WHILE expr relop expr LOOP statement-list END
input-stmt ::= INPUT var-list
binding-stmt ::= LET var = expr
exit-stmt ::= EXIT
```

## Example Program
```basic
INPUT A
WHILE a > 0 LOOP
    LET A = A - 1
    PRINT "NaN"
END
PRINT " BATMAN!"
EXIT
```

```bytecode
PUSH_CONST 0
CAPTURE_INPUT

L0:
PUSH_VAR 0
PUSH_CONST 0
GT
JMP_FALSE L1
    PUSH_VAR 0
    PUSH_CONST 1
    SUB
    PUSH_STR 0
    PRINT
    JMP L0
PUSH_STR 1
PRINT
EXIT
```