grammar UnsupportedLeftRecursive;

Identifier: [a-zA-Z_] [a-zA-Z_0-9]*;
WhiteSpace: [ \t\n\r]+ -> skip;

expr: s 'a';
s: expr 'b' | Identifier;
