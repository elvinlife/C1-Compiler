lexer grammar C1Lexer;

tokens {
    Comma,
    SemiColon,
    Assign,
    LeftBracket,
    RightBracket,
    LeftBrace,
    RightBrace,
    LeftParen,
    RightParen,
    If,
    Else,
    While,
    Const,
    Equal,
    NonEqual,
    Less,
    Greater,
    LessEqual,
    GreaterEqual,
    Plus,
    Minus,
    Multiply,
    Divide,
    Modulo,
    Int,
    Void,
    Identifier,
    Number
}

//token
Equal: '==';
If: 'if';
Else: 'else';
While: 'while';
Const: 'const';
NonEqual: '!=';
LessEqual: '<=';
GreaterEqual: '>=';
Int: 'int';
Void: 'void';
Comma: ',';
SemiColon: ';';
Assign: '=';
LeftBracket: '[';
RightBracket: ']';
LeftBrace: '{';
RightBrace: '}';
LeftParen: '(';
RightParen: ')';
Less: '<';
Greater: '>';
Plus: '+' ;
Minus: '-' ;
Multiply: '*' ;
Divide: '/' ;
Modulo: '%' ;
Identifier: Character (Character | [0-9])*;
fragment Character: [_a-zA-Z];
Number: [0-9]+ | '0x' [0-9a-fA-F]+ ;

//non-token
WhiteSpace: [ \t\r\n]+ -> skip;
LineComment: '//' (.*? ~[\\]'\r'?'\n' | '\r'?'\n') -> skip;
BlockComment: '/*' .*? '*/' -> skip;