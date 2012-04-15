
var Material = require("./material").Material;

function resolveMaterial(material) {
    var extension = material.lastIndexOf(".");
    if (extension !== -1)
        material = material.substr(0, extension);
    return material.toLowerCase();
}

exports.createOgreMaterial = function(material) {
    var result = [];
    result.push("material " + resolveMaterial(material.path) + "\n");
    result.push("{");
    result.push("\ttechnique");
    result.push("\t{");

    result.push("\t\tpass");
    result.push("\t\t{");

    /*
     Various stage properties
    */
    if (material.IsFaceCullingDisabled)
    {
       result.push("\t\t\tcull_hardware none");
       result.push("\t\t\tcull_software none");
    }

    if (material.color.join(",") !== "1,1,1,1")
    {
        result.push("\t\t\tdiffuse " + material.color.join(" "));
    }

    result.push("\t\t\tspecular 1 1 1 1 " + material.specularPower);

    if (material.depthWriteDisabled)
       result.push("\t\t\tdepth_write off");

    if (material.lightingDisabled)
       result.push("\t\t\tlighting off");

    if (material.depthTestDisabled)
       result.push("\t\t\tdepth_check off");

    if (material.recalculateNormals)
       result.push("\t\t\tnormalise_normals on");

    switch (material.blendType)
    {
       case "alpha":
           result.push("\t\t\tscene_blend alpha_blend");
           result.push("\t\t\talpha_rejection greater 0");
           break;
       case "add":
           result.push("\t\t\tscene_blend one one");
           result.push("\t\t\talpha_rejection greater 0");
           break;
       case "alphaadd":
           result.push("\t\t\tscene_blend src_alpha one");
           result.push("\t\t\talpha_rejection greater 0");
           break;
    }

    /*
    Texture Units
    */
    for (var i = 0; i < Material.TextureStageCount; ++i) {
        var stage = material.textureStage(i);

       if (!stage.filename)
           continue;

       var indent = "\t\t\t";
       result.push(indent + "texture_unit");
       result.push(indent + "{");
       result.push(indent + "\ttexture " + stage.filename + " 2d 0");

       switch (stage.uvType)
       {
       case "drift":
           result.push(indent + "\tscroll_anim " + stage.speedU + " " + stage.speedV);
           break;
       case "swirl":
           result.push(indent + "\trotate_anim " + stage.speedU);
           break;
       case "wavey":
           /* This is temporary */
           if (stage.speedU !== 0)
               result.push(indent + "\twave_xform scroll_x sine 1.0 " + stage.speedU / 60 + " 0.0 1.0");
           if (stage.speedV !== 0)
               result.push(indent + "\twave_xform scroll_y sine 1.0 " + stage.speedV / 60 + " 0.0 1.0");
           break;
       case "environment":
           result.push(indent + "\tenv_map planar");
           break;
       }

       switch (stage.blendType)
       {
           case "add":
               result.push(indent + "\tcolour_op add");
               result.push(indent + "\talpha_op_ex source1 src_current src_texture");
               break;
           case "texturealpha":
               result.push(indent + "\tcolour_op alpha_blend");
               result.push(indent + "\talpha_op_ex source1 src_current src_texture");
               break;
           case "currentalpha":
               result.push(indent + "\tcolour_op_ex blend_current_alpha src_texture src_current");
               result.push(indent + "\talpha_op_ex source1 src_diffuse src_texture");
               break;
           case "currentalphaadd":
               result.push(indent + "\tcolour_op_ex blend_current_alpha src_current src_texture");
               result.push(indent + "\talpha_op_ex source1 src_diffuse src_texture");
               break;
       }

       result.push(indent + "}");
    }

    result.push("\t\t}");

    result.push("\t}");

    result.push("}");

    return result.join("\n");

};
