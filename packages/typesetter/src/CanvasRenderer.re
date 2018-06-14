[%%debugger.chrome];
open Webapi.Canvas;

open Canvas2d;

exception NoBody;
exception NoDocument;

let makeContext = () => {
  open Webapi.Dom;
  let myCanvas = Document.createElement("canvas", document);

  myCanvas |> Element.setAttribute("width", "2048");
  myCanvas |> Element.setAttribute("height", "1200");
  myCanvas |> Element.setAttribute("style", "width:1024px; height:600px;");

  switch (Document.asHtmlDocument(document)) {
  | Some(htmlDocument) =>
    switch (HtmlDocument.body(htmlDocument)) {
    | Some(body) => body |> Element.appendChild(myCanvas)
    | _ => raise(NoBody)
    }
  | _ => raise(NoDocument)
  };

  CanvasElement.getContext2d(myCanvas);
};

let ctx = makeContext();

/* enable retina mode */
ctx |> scale(~x=2., ~y=2.);

ctx |. setFillStyle(String, "#000000");
ctx |. setStrokeStyle(String, "magenta");

module Int = {
  type t = int;
  let compare = (a, b) => Pervasives.compare(a, b);
};

module IntMap = Map.Make(Int);

let glyphMap = IntMap.empty;
let glyphMap = IntMap.add(10, "Hello", glyphMap);
let glyphMap = IntMap.add(15, "Goodbye", glyphMap);

IntMap.iter(
  (key, value) => Js.log(string_of_int(key) ++ " = " ++ value),
  glyphMap,
);

type point = {
  mutable x: float,
  mutable y: float,
};

Js.Promise.(
  Fetch.fetch("/packages/typesetter/metrics/comic-sans.json")
  |> then_(Fetch.Response.json)
  |> then_(json => {
       open Metrics;
       decodeFontData(json);

       Js.log(fontData.unitsPerEm);
       Js.log(fontData.glyphMetrics);

       ctx |. setFillStyle(String, "#0000FF");

       let fontSize = 60.;

       {
         open Layout;

         let rec layout = node : box =>
           switch (node) {
           | Node.Apply(op, args) when op == Node.Add || op == Node.Sub =>
             let boxList =
               List.fold_left(
                 (acc, arg) => {
                   let larg =
                     switch (arg) {
                     | Node.Apply(Node.Add | Node.Sub, _) =>
                       hpackNat([
                         Glyph('(', fontSize),
                         Box(0., layout(arg)),
                         Glyph(')', fontSize),
                       ])
                     | _ => layout(arg)
                     };
                   let lop = switch(op) {
                   | Node.Add => '+'
                   | Node.Sub => '-'
                   | _ => '?'
                   };
                   switch (acc) {
                   | [] => [Box(0., larg)]
                   | _ => acc @ [Glyph(lop, fontSize), Box(0., larg)]
                   };
                 },
                 [],
                 args,
               );
             hpackNat(boxList);
           | Node.Apply(Node.Div, [num, den]) =>
             let num = layout(num);
             let den = layout(den);
             let frac =
               makeFract(
                 4.5,
                 Js.Math.max_float(num.Layout.width, den.Layout.width),
                 num,
                 den,
               );
             frac;
           | Node.Number(value) =>
             hpackNat(
               Array.to_list(
                 Array.map(
                   (c: string) => Glyph(c.[0], fontSize),
                   Js.String.split("", value),
                 ),
               ),
             )
           | Node.Identifier(value) =>
             hpackNat(
               Array.to_list(
                 Array.map(
                   (c: string) => Glyph(c.[0], fontSize),
                   Js.String.split("", value),
                 ),
               ),
             )
           | _ => hpackNat([])
           };

         let tokens = Lexer.lex("(a + (b - c)) + 1 / (x + y)");
         tokens |> Array.map(Token.tokenToString) |> Array.iter(Js.log);
         let ast = MathParser.parse(tokens);

         let fract = layout(ast);

         let nl = [
           Glyph('(', fontSize),
           Glyph('a', fontSize),
           Kern(12.),
           Glyph('+', fontSize),
           Kern(12.),
           Glyph('(', fontSize),
           Glyph('b', fontSize),
           Kern(12.),
           Glyph('-', fontSize),
           Kern(12.),
           Glyph('c', fontSize),
           Glyph(')', fontSize),
           Glyph(')', fontSize),
           Kern(12.),
           Glyph('+', fontSize),
           Kern(12.),
           Box(-18., fract),
         ];

         let box = hpackNat(nl);

         ctx |. lineWidth(1.);

         Canvas2dRe.save(ctx);
         Canvas2dRe.translate(~x=200., ~y=400., ctx);
         Renderer.render(ctx, box, 0.);
         ctx |. setFillStyle(String, "#000000");
         ctx |. fillRect(~x=0., ~y=0., ~w=5., ~h=5.);
         Canvas2dRe.restore(ctx);

         Canvas2dRe.save(ctx);
         Canvas2dRe.translate(~x=200., ~y=200., ctx);
         Renderer.render(ctx, hpackNat([Box(-18., fract)]), 0.);
         ctx |. setFillStyle(String, "#000000");
         ctx |. fillRect(~x=0., ~y=0., ~w=5., ~h=5.);
         Canvas2dRe.restore(ctx);
       };

       resolve();
     })
);

let tokens = Lexer.lex("1+2+3");
let ast = MathParser.parse(tokens);

let layout = node =>
  switch (node) {
  | Node.Apply(_, _) => ()
  | Identifier(_) => ()
  | Number(_) => ()
  | Ellipses => ()
  };

Js.log(Node.toString(ast));