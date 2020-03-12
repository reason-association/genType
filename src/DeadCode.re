open DeadCommon;

let (+++) = Filename.concat;

let getModuleName = fn => fn |> Paths.getModuleName |> ModuleName.toString;

let rec getSignature = (~isfunc=false, moduleType: Types.module_type) =>
  switch (moduleType) {
  | Mty_signature(signature) => signature
  | Mty_functor(_, tOpt, _) when isfunc =>
    switch (tOpt) {
    | None => []
    | Some(moduleType) => getSignature(moduleType)
    }
  | Mty_functor(_, _, moduleType) => getSignature(moduleType)
  | _ => []
  };

let rec collectExportFromSignatureItem = (~path, si: Types.signature_item) =>
  switch (si) {
  | Sig_value(id, {Types.val_loc, val_kind}) when !val_loc.Location.loc_ghost =>
    let isPrimitive =
      switch (val_kind) {
      | Val_prim(_) => true
      | _ => false
      };
    if (!isPrimitive || analyzeExternals) {
      addDeclaration(
        ~declKind=Value,
        ~path,
        ~loc=val_loc,
        ~name=Ident.name(id),
      );
    };
  | Sig_type(id, t, _) =>
    if (analyzeTypes^) {
      DeadType.addTypeDeclaration(~path=[id |> Ident.name, ...path], t);
    }
  | (
      Sig_module(id, {Types.md_type: moduleType}, _) |
      Sig_modtype(id, {Types.mtd_type: Some(moduleType)})
    ) as s =>
    let collect =
      switch (s) {
      | Sig_modtype(_) => false
      | _ => true
      };
    if (collect) {
      getSignature(moduleType)
      |> List.iter(
           collectExportFromSignatureItem(~path=[id |> Ident.name, ...path]),
         );
    };
  | _ => ()
  };

let processSignature = (signature: Types.signature) => {
  let module_id = currentModuleName^;
  signature
  |> List.iter(sig_item =>
       collectExportFromSignatureItem(~path=[module_id], sig_item)
     );
};

let loadCmtFile = cmtFilePath => {
  if (verbose) {
    Log_.item("Scanning %s@.", cmtFilePath);
  };

  let {Cmt_format.cmt_annots, cmt_sourcefile, cmt_value_dependencies} =
    Cmt_format.read_cmt(cmtFilePath);

  switch (cmt_sourcefile) {
  | None => ()

  | Some(sourceFile_) =>
    let sourceFile =
      if (Filename.check_suffix(sourceFile_, ".re.ml")) {
        Filename.chop_suffix(sourceFile_, ".ml");
      } else if (Filename.check_suffix(sourceFile_, ".re.mli")) {
        Filename.chop_suffix(sourceFile_, ".re.mli") ++ ".rei";
      } else {
        sourceFile_;
      };

    FileHash.addFile(fileReferences, sourceFile);
    currentSrc := sourceFile;
    currentModuleName := getModuleName(sourceFile);

    if (dce^) {
      switch (cmt_annots) {
      | Interface(signature) =>
        ProcessDeadAnnotations.signature(signature);
        processSignature(signature.sig_type);
      | Implementation(structure) =>
        let cmtiExists =
          Sys.file_exists(
            (cmtFilePath |> Filename.chop_extension) ++ ".cmti",
          );
        if (!cmtiExists) {
          ProcessDeadAnnotations.structure(structure);
          processSignature(structure.str_type);
        };
        DeadValue.processStructure(~cmt_value_dependencies, structure);
      | _ => ()
      };
    };
    if (analyzeTermination^) {
      switch (cmt_annots) {
      | Interface(_) => ()
      | Implementation(structure) => Arnold.processStructure(structure)
      | _ => ()
      };
    };
  };
};

let reportResults = () => {
  let ppf = Format.std_formatter;
  let onItem = ({declKind, pos, path}) => {
    let loc = {Location.loc_start: pos, loc_end: pos, loc_ghost: false};
    let (name, message) =
      switch (declKind) {
      | Value => (
          "Warning Dead Value",
          switch (path) {
          | ["_", ..._] => "has no side effects and can be removed"
          | _ => "is never used"
          },
        )
      | RecordLabel => (
          "Warning Dead Type",
          "is a record label never used to read a value",
        )
      | VariantCase => (
          "Warning Dead Type",
          "is a variant case which is never constructed",
        )
      };
    Log_.info(~loc, ~name, (ppf, ()) =>
      Format.fprintf(ppf, "@{<info>%s@} %s", path |> pathWithoutHead, message)
    );
  };
  let onDeadCode = item => {
    item |> onItem;
    item |> WriteDeadAnnotations.onDeadItem(~ppf);
  };
  reportDead(~onDeadCode);
  WriteDeadAnnotations.write();
};

let runAnalysis = (~cmtRoot) => {
  Log_.Color.setup();
  switch (cmtRoot) {
  | Some(root) =>
    let rec walkSubDirs = dir => {
      let absDir = dir == "" ? root : root +++ dir;
      if (Sys.file_exists(absDir)) {
        if (Sys.is_directory(absDir)) {
          absDir |> Sys.readdir |> Array.iter(d => walkSubDirs(dir +++ d));
        } else if (Filename.check_suffix(absDir, ".cmt")
                   || Filename.check_suffix(absDir, ".cmti")) {
          absDir |> loadCmtFile;
        };
      };
    };
    walkSubDirs("");
    if (dce^) {
      reportResults();
    };
    if (analyzeTermination^) {
      Arnold.reportResults();
    };

  | None =>
    Paths.setProjectRoot();
    let lib_bs = {
      GenTypeCommon.projectRoot^ +++ "lib" +++ "bs";
    };

    let sourceDirs = ModuleResolver.readSourceDirs(~configSources=None);
    sourceDirs.dirs
    |> List.iter(sourceDir => {
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
         cmtFiles
         |> List.iter(cmtFile => {
              let cmtFilePath = Filename.concat(libBsSourceDir, cmtFile);
              cmtFilePath |> loadCmtFile;
            });
       });

    if (dce^) {
      reportResults();
    };
    if (analyzeTermination^) {
      Arnold.reportResults();
    };
  };
};

let runTerminationAnalysis = (~cmtRoot) => {
  dce := false;
  analyzeTermination := true;
  runAnalysis(~cmtRoot);
};