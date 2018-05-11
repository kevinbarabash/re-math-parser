type operator =
  | Add
  | Mul
  | Neg
  | Pos
  | Div
  | Exp;

let opToString = op =>
  switch (op) {
  | Add => "+"
  | Mul => "*"
  | Neg => "neg"
  | Pos => "pos"
  | Div => "/"
  | Exp => "^"
  };

let getOpPrecedence = op =>
  switch (op) {
  | Add => 10
  | Mul => 20
  | Div => 20
  | Exp => 30
  | Pos => 25
  | Neg => 25
  };

type node =
  | Apply(operator, list(node))
  | Identifier(string)
  | Number(string);

exception Error;

let parse = (tokens, src: string) => {
  let peek = () =>
    Lexer.(
      try (tokens[0]) {
      | Invalid_argument("index out of bounds") => {
          t: EOF,
          value: "",
          loc: {
            start: (-1),
            end_: (-1),
          },
        }
      }
    );
  let getPrecedence = () =>
    Lexer.(
      /* This is only necessary for handle infix operators */
      switch (peek().t) {
      | PLUS => 10
      | STAR => 20
      | SLASH => 20
      | CARET => 30
      | _ => 0
      }
    );
  let consume = () =>
    Lexer.(
      switch (Js.Array.shift(tokens)) {
      | Some(token) => token
      | None => {
          t: EOF,
          value: "",
          loc: {
            start: (-1),
            end_: (-1),
          },
        }
      }
    );
  let rec getPrefixParselet = token =>
    Lexer.(
      switch (token.t) {
      | IDENTIFIER(name) => Some(() => Identifier(name))
      | NUMBER(value) => Some(() => Number(value))
      | MINUS => Some(parsePrefix(Neg))
      | PLUS => Some(parsePrefix(Pos))
      | _ => None
      }
    )
  and getInfixParselet = token =>
    Lexer.(
      switch (token.t) {
      | PLUS => Some(parseNaryInfix(Add))
      | STAR => Some(parseNaryInfix(Mul))
      | SLASH => Some(parseBinaryInfix(Div))
      | CARET => Some(parseBinaryInfix(Exp))
      | _ => None
      }
    )
  and _parse = (precedence: int) => {
    let left =
      switch (getPrefixParselet(consume())) {
      | Some(parselet) => parselet()
      | None => raise(Error)
      };
    switch (getInfixParselet(peek())) {
    | Some(parselet) => parselet(precedence, left)
    | None => left
    };
  }
  and parsePrefix = (op, ()) => Apply(Neg, [_parse(getOpPrecedence(op))])
  and parseNaryInfix = (op, precedence, left) =>
    switch (parseNaryArgs(op, precedence, peek())) {
    | [] => left
    | otherArgs => Apply(op, [left] @ otherArgs)
    }
  and parseNaryArgs = (op, precedence, token) : list(node) =>
    Lexer.(
      if (precedence < getPrecedence()) {
        consume() |> ignore;
        let result = _parse(getOpPrecedence(op));
        token.t == peek().t ?
          [result] @ parseNaryArgs(op, precedence, token) : [result];
      } else {
        [];
      }
    )
  and parseBinaryInfix = (op, precedence, left) : node =>
    if (precedence < getPrecedence()) {
      switch (getInfixParselet(consume())) {
      | Some(parselet) =>
        parselet(
          precedence,
          Apply(op, [left, _parse(getOpPrecedence(op))]),
        )
      | None => raise(Error)
      };
    } else {
      left;
    };
  _parse(0);
};

let rec nodeToString = node =>
  switch (node) {
  | Apply(op, children) =>
    "["
    ++ Js.Array.joinWith(
         " ",
         Array.of_list([
           opToString(op),
           ...List.map(nodeToString, children),
         ]),
       )
    ++ "]"
  | Identifier(name) => name
  | Number(value) => value
  };
