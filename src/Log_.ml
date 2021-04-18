module Color = struct
  let color_enabled = lazy (Unix.isatty Unix.stdout)

  let forceColor = ref false

  let get_color_enabled () = !forceColor || Lazy.force color_enabled

  type color = Red | Yellow | Magenta | Cyan

  type style = FG of color | Bold | Dim

  let code_of_style = function
    | FG Red -> "31"
    | FG Yellow -> "33"
    | FG Magenta -> "35"
    | FG Cyan -> "36"
    | Bold -> "1"
    | Dim -> "2"

  let style_of_tag s =
    match s with
    | "error" -> [Bold; FG Red]
    | "warning" -> [Bold; FG Magenta]
    | "info" -> [Bold; FG Yellow]
    | "dim" -> [Dim]
    | "filename" -> [FG Cyan]
    | _ -> []

  let ansi_of_tag s =
    let l = style_of_tag s in
    let s = String.concat ";" (List.map code_of_style l) in
    "\027[" ^ s ^ "m"

  let reset_lit = "\027[0m"

  let color_functions =
    (({
        mark_open_tag =
          (fun s -> if get_color_enabled () then ansi_of_tag s else "");
        mark_close_tag =
          (fun _ -> if get_color_enabled () then reset_lit else "");
        print_open_tag = (fun _ -> ());
        print_close_tag = (fun _ -> ());
      }
       : Format.formatter_tag_functions)
      : Format.formatter_tag_functions)

  let setup () =
    Format.pp_set_mark_tags Format.std_formatter true;
    Format.pp_set_formatter_tag_functions Format.std_formatter color_functions

  let error ppf s = Format.fprintf ppf "@{<error>%s@}" s [@@dead "Color.error"]

  let info ppf s = Format.fprintf ppf "@{<info>%s@}" s
end

module Loc = struct
  let print_filename ppf file =
    match file with
    (* modified *)
    | "_none_" | "" -> Format.fprintf ppf "(No file name)"
    | real_file -> Format.fprintf ppf "%s" (Location.show_filename real_file)

  let print_loc ~normalizedRange ppf (loc : Location.t) =
    let file, _, _ = Location.get_pos_info loc.loc_start in
    if file = "//toplevel//" then
      if Location.highlight_locations ppf [loc] then ()
      else
        Format.fprintf ppf "Characters %i-%i" loc.loc_start.pos_cnum
          loc.loc_end.pos_cnum
    else
      let dim_loc ppf = function
        | None -> ()
        | Some
            ((start_line, start_line_start_char), (end_line, end_line_end_char))
          ->
          if start_line = end_line then
            if start_line_start_char = end_line_end_char then
              Format.fprintf ppf ":@{<dim>%i:%i@}" start_line
                start_line_start_char
            else
              Format.fprintf ppf ":@{<dim>%i:%i-%i@}" start_line
                start_line_start_char end_line_end_char
          else
            Format.fprintf ppf ":@{<dim>%i:%i-%i:%i@}" start_line
              start_line_start_char end_line end_line_end_char
      in
      Format.fprintf ppf "@{<filename>%a@}%a" print_filename file dim_loc
        normalizedRange

  let print ppf (loc : Location.t) =
    let _file, start_line, start_char = Location.get_pos_info loc.loc_start in
    let _, end_line, end_char = Location.get_pos_info loc.loc_end in
    let normalizedRange =
      if start_char == -1 || end_char == -1 then None
      else if start_line = end_line && start_char >= end_char then
        let same_char = start_char + 1 in
        Some ((start_line, same_char), (end_line, same_char))
      else Some ((start_line, start_char + 1), (end_line, end_char))
    in

    Format.fprintf ppf "@[%a@]" (print_loc ~normalizedRange) loc
end

let item x =
  Format.fprintf Format.std_formatter "  ";
  Format.fprintf Format.std_formatter x

let logKind body ~color ~loc ~name =
  Format.fprintf Format.std_formatter "@[<v 2>@,%a@,%a@,%a@]@." color name
    Loc.print loc body ()

let info body ~loc ~name = logKind body ~color:Color.info ~loc ~name
