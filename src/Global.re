include GenTypeCommon;

let active =
  switch (Sys.getenv("Global")) {
  | _ => true
  | exception _ => false
  };

let processCmt = (~sourceDir, cmtFile) => {
  let _inputCMT = Cmt_format.read_cmt(Filename.concat(sourceDir, cmtFile));
  logItem("READ cmtFile: %s\n", cmtFile);
};

if (active) {
  logItem("Global!\n");

  Paths.setProjectRoot();
  let lib_bs = {
    let (++) = Filename.concat;
    projectRoot^ ++ "lib" ++ "bs";
  };
  logItem("lib_bs: %s\n", lib_bs);

  let sourceDirs =
    ModuleResolver.readSourceDirs() |> List.map(Filename.concat(lib_bs));
  sourceDirs
  |> List.iter(sourceDir => {
       let files =
         switch (Sys.readdir(sourceDir) |> Array.to_list) {
         | files => files
         | exception (Sys_error(_)) => []
         };
       let cmtFiles =
         files |> List.filter(x => Filename.check_suffix(x, ".cmt"));
       cmtFiles |> List.iter(processCmt(~sourceDir));
     });
};