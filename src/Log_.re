module Color = {
  let color_enabled = lazy(Unix.isatty(Unix.stdout));

  let get_color_enabled = () => {
    Lazy.force(color_enabled);
  };

  type color =
    | Black
    | Red
    | Green
    | Yellow
    | Blue
    | Magenta
    | Cyan
    | White;

  type style =
    | FG(color)
    | BG(color)
    | Bold
    | Dim;

  let code_of_style =
    fun
    | FG(Black) => "30"
    | FG(Red) => "31"
    | FG(Green) => "32"
    | FG(Yellow) => "33"
    | FG(Blue) => "34"
    | FG(Magenta) => "35"
    | FG(Cyan) => "36"
    | FG(White) => "37"

    | BG(Black) => "40"
    | BG(Red) => "41"
    | BG(Green) => "42"
    | BG(Yellow) => "43"
    | BG(Blue) => "44"
    | BG(Magenta) => "45"
    | BG(Cyan) => "46"
    | BG(White) => "47"

    | Bold => "1"
    | Dim => "2";

  let style_of_tag = s =>
    switch (s) {
    | "error" => [Bold, FG(Red)]
    | "warning" => [Bold, FG(Magenta)]
    | "info" => [Bold, FG(Yellow)]
    | "dim" => [Dim]
    | "filename" => [FG(Cyan)]
    | _ => []
    };

  let ansi_of_tag = s => {
    let l = style_of_tag(s);
    let s = String.concat(";", List.map(code_of_style, l));
    "\027[" ++ s ++ "m";
  };

  let reset_lit = "\027[0m";

  let color_functions: Format.formatter_tag_functions = (
    {
      mark_open_tag: s =>
        if (get_color_enabled()) {
          ansi_of_tag(s);
        } else {
          "";
        },
      mark_close_tag: _ =>
        if (get_color_enabled()) {
          reset_lit;
        } else {
          "";
        },
      print_open_tag: _ => (),
      print_close_tag: _ => (),
    }: Format.formatter_tag_functions
  );

  let setup = () => {
    Format.pp_set_mark_tags(Format.std_formatter, true);
    Format.pp_set_formatter_tag_functions(
      Format.std_formatter,
      color_functions,
    );
  };

  let error = (ppf, s) => Format.fprintf(ppf, "@{<error>%s@}", s);
  let info = (ppf, s) => Format.fprintf(ppf, "@{<info>%s@}", s);
  let warning = (ppf, s) => Format.fprintf(ppf, "@{<warning>%s@}", s);
};

module Loc = {
  let print_filename = (ppf, file) =>
    switch (file) {
    /* modified */
    | "_none_"
    | "" => Format.fprintf(ppf, "(No file name)")
    | real_file =>
      Format.fprintf(ppf, "%s", Location.show_filename(real_file))
    };

  let print_loc = (~normalizedRange, ppf, loc: Location.t) => {
    let (file, _, _) = Location.get_pos_info(loc.loc_start);
    if (file == "//toplevel//") {
      if (Location.highlight_locations(ppf, [loc])) {
        ();
      } else {
        Format.fprintf(
          ppf,
          "Characters %i-%i",
          loc.loc_start.pos_cnum,
          loc.loc_end.pos_cnum,
        );
      };
    } else {
      let dim_loc = ppf =>
        fun
        | None => ()
        | Some((
            (start_line, start_line_start_char),
            (end_line, end_line_end_char),
          )) =>
          if (start_line == end_line) {
            if (start_line_start_char == end_line_end_char) {
              Format.fprintf(
                ppf,
                " @{<dim>%i:%i@}",
                start_line,
                start_line_start_char,
              );
            } else {
              Format.fprintf(
                ppf,
                " @{<dim>%i:%i-%i@}",
                start_line,
                start_line_start_char,
                end_line_end_char,
              );
            };
          } else {
            Format.fprintf(
              ppf,
              " @{<dim>%i:%i-%i:%i@}",
              start_line,
              start_line_start_char,
              end_line,
              end_line_end_char,
            );
          };

      Format.fprintf(
        ppf,
        "@{<filename>%a@}%a",
        print_filename,
        file,
        dim_loc,
        normalizedRange,
      );
    };
  };

  let print = (ppf, loc: Location.t) => {
    let (file, start_line, start_char) =
      Location.get_pos_info(loc.loc_start);
    let (_, end_line, end_char) = Location.get_pos_info(loc.loc_end);
    let normalizedRange =
      if (start_char === (-1) || end_char === (-1)) {
        None;
      } else if (start_line == end_line && start_char >= end_char) {
        let same_char = start_char + 1;
        Some(((start_line, same_char), (end_line, same_char)));
      } else {
        Some(((start_line, start_char + 1), (end_line, end_char)));
      };
    Format.fprintf(ppf, "@[%a@]", print_loc(~normalizedRange), loc);
  };
};

let log = x => {
  Format.fprintf(Format.std_formatter, x);
};

let item = x => {
  Format.fprintf(Format.std_formatter, "  ");
  Format.fprintf(Format.std_formatter, x);
};

let logKind = (body, ~color, ~loc, ~name) => {
  Format.fprintf(
    Format.std_formatter,
    "@[<v 2>@,%a@,%a@,%a@]@.",
    color,
    name,
    Loc.print,
    loc,
    body,
    (),
  );
};

let warning = (body, ~loc, ~name) =>
  logKind(body, ~color=Color.warning, ~loc, ~name);
let info = (body, ~loc, ~name) =>
  logKind(body, ~color=Color.info, ~loc, ~name);
let error = (body, ~loc, ~name) =>
  logKind(body, ~color=Color.error, ~loc, ~name);