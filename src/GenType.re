/***
 * Copyright 2004-present Facebook. All Rights Reserved.
 */

open GenTypeCommon;

let version = Version.version;

let signFile = s => s;

type cliCommand =
  | Add(string)
  | Clean
  | NoOp
  | Rm(string);

let cli = () => {
  let bsVersion = ref(None);
  let cliCommand = ref(NoOp);
  let setBsVersion = s => {
    bsVersion := Some(s);
  };
  let usage = "genType version " ++ version;
  let versionAndExit = () => {
    print_endline(usage);
    exit(0);
  };
  let rec printUsageAndExit = () => {
    Arg.usage(speclist, usage);
    exit(0);
  }
  and setCliCommand = command => {
    if (cliCommand^ != NoOp) {
      printUsageAndExit();
    };
    cliCommand := command;
  }
  and setAdd = s => {
    Add(s) |> setCliCommand;
  }
  and setRm = s => {
    Rm(s) |> setCliCommand;
  }
  and setClean = () => {
    Clean |> setCliCommand;
  }
  and speclist = [
    (
      "-bs-version",
      Arg.String(setBsVersion),
      "set the bucklescript version",
    ),
    ("-clean", Arg.Unit(setClean), "clean all the generated files"),
    ("-cmt-add", Arg.String(setAdd), "compile a .cmt[i] file"),
    ("-cmt-rm", Arg.String(setRm), "remove a .cmt[i] file"),
    (
      "-version",
      Arg.Unit(versionAndExit),
      "show version information and exit",
    ),
    (
      "--version",
      Arg.Unit(versionAndExit),
      "show version information and exit",
    ),
  ];

  let executeCliCommand = (~bsVersion, cliCommand) =>
    switch (cliCommand) {
    | Add(s) =>
      let splitColon = Str.split(Str.regexp(":"), s);
      let (cmt, mlast) =
        switch (splitColon) {
        | [cmt, ...rest] =>
          let mlast = rest |> String.concat("");
          (cmt, mlast);
        | _ => assert(false)
        };
      let config =
        Paths.readConfig(~bsVersion, ~namespace=cmt |> Paths.findNameSpace);
      if (Debug.basic^) {
        logItem("Add %s  %s\n", cmt, mlast);
      };
      cmt |> GenTypeMain.processCmtFile(~signFile, ~config);
      exit(0);

    | Clean =>
      let config = Paths.readConfig(~bsVersion, ~namespace=None);
      let dirs = ModuleResolver.readSourceDirs();
      if (Debug.basic^) {
        logItem("Clean %d dirs\n", dirs |> List.length);
      };
      let count = ref(0);
      dirs
      |> List.iter(dir => {
           let files = Sys.readdir(dir);
           files
           |> Array.iter(file =>
                if (Filename.check_suffix(file, ".re")) {
                  let extension = EmitType.outputFileSuffix(~config);
                  let generated =
                    Filename.concat(
                      dir,
                      (file |> Filename.chop_extension) ++ extension,
                    );
                  if (Sys.file_exists(generated)) {
                    Unix.unlink(generated);
                    incr(count);
                  };
                }
              );
         });
      if (Debug.basic^) {
        logItem("Cleaned %d files\n", count^);
      };
      exit(0);

    | NoOp => printUsageAndExit()

    | Rm(s) =>
      let splitColon = Str.split(Str.regexp(":"), s) |> Array.of_list;
      assert(Array.length(splitColon) === 1);
      let cmtAbsolutePath: string = splitColon[0];
      /* somehow the CMT hook is passing an absolute path here */
      let cmt = cmtAbsolutePath |> Paths.relativePathFromBsLib;
      let config =
        Paths.readConfig(~bsVersion, ~namespace=cmt |> Paths.findNameSpace);
      let outputFile = cmt |> Paths.getOutputFile(~config);
      if (Debug.basic^) {
        logItem("Remove %s\n", cmt);
      };
      if (Sys.file_exists(outputFile)) {
        Unix.unlink(outputFile);
      };
      exit(0);
    };

  Arg.parse(speclist, print_endline, usage);

  executeCliCommand(~bsVersion=bsVersion^, cliCommand^);
};

if (DeadCommon.active) {
  DeadCode.runAnalysis();
} else {
  cli();
};