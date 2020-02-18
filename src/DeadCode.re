open DeadCommon;

let (+++) = Filename.concat;

let getModuleName = fn => fn |> Paths.getModuleName |> ModuleName.toString;

let loadFile = (~sourceFile, cmtFilePath) => {
  lastPos := Lexing.dummy_pos;
  if (verbose) {
    GenTypeCommon.logItem("Scanning %s\n", cmtFilePath);
  };
  currentSrc := sourceFile;
  currentModuleName := getModuleName(sourceFile);

  let {Cmt_format.cmt_annots, cmt_value_dependencies} =
    Cmt_format.read_cmt(cmtFilePath);

  if (dce^) {
    switch (cmt_annots) {
    | Interface(signature) =>
      ProcessDeadAnnotations.signature(signature);
      DeadValue.processSignature(cmtFilePath, signature.sig_type);
    | Implementation(structure) =>
      let cmtiExists =
        Sys.file_exists((cmtFilePath |> Filename.chop_extension) ++ ".cmti");
      if (!cmtiExists) {
        ProcessDeadAnnotations.structure(structure);
      };
      DeadValue.processStructure(
        ~cmtiExists,
        cmt_value_dependencies,
        structure,
      );
      if (!cmtiExists) {
        DeadValue.processSignature(cmtFilePath, structure.str_type);
      };
    | _ => ()
    };
  };
  if (analyzeTermination^) {
    switch (cmt_annots) {
    | Interface(signature) => ()
    | Implementation(structure) => Arnold.processStructure(structure)
    | _ => ()
    };
  };
};

let reportResults = (~posInAliveWhitelist) => {
  let ppf = Format.std_formatter;
  let onItem = ({analysisKind, pos, path}) => {
    let loc = {Location.loc_start: pos, loc_end: pos, loc_ghost: false};
    let name =
      switch (analysisKind) {
      | Value => "Warning Dead Value"
      | Type => "Warning Dead Type"
      };
    GenTypeCommon.logInfo(~loc, ~name, (ppf, ()) =>
      Format.fprintf(ppf, "@{<info>%s@} is never used", path)
    );
  };
  let onDeadCode = item => {
    item |> onItem;
    item |> WriteDeadAnnotations.onDeadItem(~ppf);
  };
  reportDead(~onDeadCode, ~posInAliveWhitelist);
  WriteDeadAnnotations.write();
};

let processCmt = (~libBsSourceDir, ~sourceDir, cmtFile) => {
  let extension = Filename.extension(cmtFile);
  let moduleName = cmtFile |> getModuleName;
  let sourceFile = ext =>
    (GenTypeCommon.projectRoot^ +++ sourceDir +++ moduleName) ++ ext;
  let sourceFileRE = sourceFile(extension == ".cmti" ? ".rei" : ".re");
  let sourceFileML = sourceFile(extension == ".cmti" ? ".mli" : ".ml");
  let sourceFile =
    if (!Sys.file_exists(sourceFileRE)) {
      if (!Sys.file_exists(sourceFileML)) {
        GenTypeCommon.logItem(
          "XXX sourceFile does not exist: %s\n",
          Filename.basename(sourceFileRE),
        );
        assert(false);
      };
      sourceFileML;
    } else {
      sourceFileRE;
    };

  FileHash.addFile(fileReferences, sourceFile);
  let cmtFilePath = Filename.concat(libBsSourceDir, cmtFile);
  cmtFilePath |> loadFile(~sourceFile);
};

let aliveWhitelist = ["DeadTestWhitelist"];
let aliveWhitelistTable = {
  let tbl = Hashtbl.create(1);
  List.iter(
    moduleName => Hashtbl.replace(tbl, moduleName, ()),
    aliveWhitelist,
  );
  tbl;
};

let posInAliveWhitelist = (pos: Lexing.position) => {
  let moduleName = pos.pos_fname |> getModuleName;
  Hashtbl.mem(aliveWhitelistTable, moduleName);
};

let runAnalysis = (~cmtFileOpt) => {
  if (dce^ || analyzeTermination^) {
    GenTypeCommon.Color.setup();
  };
  switch (cmtFileOpt, analyzeTermination^) {
  | (Some(cmtFile), true) =>
    let cmtFilePath = cmtFile;
    let sourceFile = Filename.basename(cmtFile) ++ ".ml";
    cmtFilePath |> loadFile(~sourceFile);
    Arnold.reportResults();
  | _ =>
    Paths.setProjectRoot();
    let lib_bs = {
      GenTypeCommon.projectRoot^ +++ "lib" +++ "bs";
    };

    let sourceDirs = ModuleResolver.readSourceDirs(~configSources=None);
    sourceDirs.dirs
    |> List.iter(sourceDir =>
         if (sourceDir |> whitelistSourceDir) {
           let libBsSourceDir = Filename.concat(lib_bs, sourceDir);
           let files =
             switch (Sys.readdir(libBsSourceDir) |> Array.to_list) {
             | files => files
             | exception (Sys_error(_)) => []
             };
           let cmtFiles =
             files
             |> List.filter(x =>
                  Filename.check_suffix(x, ".cmt")
                  || Filename.check_suffix(x, ".cmti")
                );
           cmtFiles |> List.iter(processCmt(~libBsSourceDir, ~sourceDir));
         }
       );

    if (dce^) {
      reportResults(~posInAliveWhitelist);
    };
    if (analyzeTermination^) {
      Arnold.reportResults();
    };
  };
};

let runTerminationAnalysis = (~cmtFileOpt) => {
  dce := false;
  analyzeTermination := true;
  runAnalysis(~cmtFileOpt);
};