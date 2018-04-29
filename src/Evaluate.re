let sum = Array.fold_left((+.), 0.);
let prod = Array.fold_left(( *. ), 1.);

let rec evaluate =
  Parser.(
    node =>
      switch (node) {
      | Apply(op, children) =>
        let children = Array.map(evaluate, children);
        switch (op) {
        | Add => children |> sum
        | Mul(_) => children |> prod
        | _ => 0.
        };
      | Identifier(_) => 0. /* allow option to provide a map of values */
      | Number(value) => float_of_string(value)
      }
  );
  