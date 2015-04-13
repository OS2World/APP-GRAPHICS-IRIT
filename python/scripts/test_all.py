#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
import time
import os
#

irit.iritstate( "DoGraphics", irit.GenRealObject( 0 ) )

# 
#  Test file to include ALL scripts without any pause.
# 
#

def empty_function():
    pass
       
irit.pause = empty_function

def print_time( file_name, a, b):
    total_time = b - a
    print "************* Execution Time for %s is %f ******************\\n" % ( file_name , total_time)
    
def work_function(file_name):
    a = time.time()
    execfile(file_name, {'_main_':1} )
    b = time.time()
    print_time(file_name ,a , b)


file_name = "adap_iso.py"
work_function(file_name)

file_name = "algsum.py"
work_function(file_name)

file_name = "alphsec2.py"
work_function(file_name)

file_name = "ambiguit.py"
work_function(file_name)

file_name = "alphsect.py"
work_function(file_name)

file_name = "analyfit.py"
work_function(file_name)

file_name = "animball.py"
work_function(file_name)

file_name = "animdemo.py"
work_function(file_name)

file_name = "ant.py"
work_function(file_name)

file_name = "antipodl.py"
work_function(file_name)

file_name = "arc_len.py"
work_function(file_name)

file_name = "art_glry.py"
work_function(file_name)

file_name = "aspct_gr.py"
work_function(file_name)

file_name = "b58.py"
work_function(file_name)

file_name = "bed.py"
work_function(file_name)

file_name = "bez_clip.py"
work_function(file_name)

file_name = "bezier.py"
work_function(file_name)

file_name = "bisectrs.py"
work_function(file_name)

file_name = "biarc.py"
work_function(file_name)

file_name = "blending.py"
work_function(file_name)

file_name = "blossom.py"
work_function(file_name)

file_name = "box-box.py"
work_function(file_name)

file_name = "bsc_geom.py"
work_function(file_name)

file_name = "bsct_smp.py"
work_function(file_name)

file_name = "bspline.py"
work_function(file_name)

file_name = "bulb.py"
work_function(file_name)

file_name = "cbi2anim.py"
work_function(file_name)

file_name = "cbi_anim.py"
work_function(file_name)

file_name = "cbisec3d.py"
work_function(file_name)

file_name = "cbisect2.py"
work_function(file_name)

file_name = "cbisect.py"
work_function(file_name)

file_name = "cbsp_fit.py"
work_function(file_name)

file_name = "cenvoff.py"
work_function(file_name)

file_name = "chess.py"
work_function(file_name)

#file_name = "clntcrsr.py"
#work_function(file_name)

file_name = "closloop.py"
work_function(file_name)

file_name = "cmorph2.py"
work_function(file_name)

file_name = "cmorph.py"
work_function(file_name)

file_name = "cnc_ltrs.py"
work_function(file_name)

file_name = "colors.py"
work_function(file_name)

file_name = "cone2cyl.py"
work_function(file_name)

file_name = "cone3cyl.py"
work_function(file_name)

file_name = "cone-cyl.py"
work_function(file_name)

file_name = "conicdst.py"
work_function(file_name)

file_name = "conics.py"
work_function(file_name)

file_name = "contours.py"
work_function(file_name)

file_name = "coords.py"
work_function(file_name)

file_name = "coplanar.py"
work_function(file_name)

file_name = "corkplug.py"
work_function(file_name)

file_name = "cort_map.py"
work_function(file_name)

file_name = "cos_sphr.py"
work_function(file_name)

file_name = "crosplug.py"
work_function(file_name)

file_name = "crv_dist.py"
work_function(file_name)

file_name = "crv_krnl.py"
work_function(file_name)

file_name = "crv_tan.py"
work_function(file_name)

file_name = "crvarrng.py"
work_function(file_name)

file_name = "crvdecmp.py"
work_function(file_name)

file_name = "crvtrrec.py"
work_function(file_name)

file_name = "cslogo.py"
work_function(file_name)

file_name = "cube.py"
work_function(file_name)

file_name = "cube2.py"
work_function(file_name)

file_name = "cube3cut.py"
work_function(file_name)

file_name = "cubes.py"
work_function(file_name)

file_name = "cup.py"
work_function(file_name)

file_name = "curvatur.py"
work_function(file_name)

file_name = "cvisible.py"
work_function(file_name)

file_name = "cylin3.py"
work_function(file_name)

file_name = "decimate.py"
work_function(file_name)

file_name = "dinner.py"
work_function(file_name)

file_name = "disjoint.py"
work_function(file_name)

file_name = "disp_map.py"
work_function(file_name)

file_name = "dist2ff.py"
work_function(file_name)

file_name = "distmtch.py"
work_function(file_name)

file_name = "domino.py"
work_function(file_name)

file_name = "duality.py"
work_function(file_name)

file_name = "duck.py"
work_function(file_name)

file_name = "ellipses.py"
work_function(file_name)

file_name = "escher.py"
work_function(file_name)

file_name = "f16.py"
work_function(file_name)

file_name = "facemask.py"
work_function(file_name)

file_name = "ffcmpcrv.py"
work_function(file_name)

file_name = "ffcnvhul.py"
work_function(file_name)

file_name = "ffloops.py"
work_function(file_name)

file_name = "ffmatch4.py"
work_function(file_name)

file_name = "ffptdist.py"
work_function(file_name)

file_name = "flecnodl.py"
work_function(file_name)

file_name = "flippage.py"
work_function(file_name)

file_name = "freeform.py"
work_function(file_name)

file_name = "function.py"
work_function(file_name)

file_name = "gamakrnl.py"
work_function(file_name)

file_name = "gearbox.py"
work_function(file_name)

file_name = "gersktch.py"
work_function(file_name)

file_name = "glocal.py"
work_function(file_name)

file_name = "handset.py"
work_function(file_name)

file_name = "hausdorf.py"
work_function(file_name)

file_name = "hermite.py"
work_function(file_name)

file_name = "interpol.py"
work_function(file_name)

file_name = "isocline.py"
work_function(file_name)

file_name = "knight.py"
work_function(file_name)

file_name = "knotrmvl.py"
work_function(file_name)

file_name = "knots.py"
work_function(file_name)

file_name = "lightmil.py"
work_function(file_name)

file_name = "lj8000.py"
work_function(file_name)

file_name = "ln2circ.py"
work_function(file_name)

file_name = "loffset2.py"
work_function(file_name)

file_name = "loffset.py"
work_function(file_name)

file_name = "logos.py"
work_function(file_name)

file_name = "loops.py"
work_function(file_name)

file_name = "macros.py"
work_function(file_name)

file_name = "make_spr.py"
work_function(file_name)

file_name = "mbisect.py"
work_function(file_name)

file_name = "min_dist.py"
work_function(file_name)

file_name = "molecule.py"
work_function(file_name)

file_name = "moments.py"
work_function(file_name)

file_name = "mrchcube.py"
work_function(file_name)

file_name = "mrescrv.py"
work_function(file_name)

file_name = "msc_ch.py"
work_function(file_name)

file_name = "multivar.py"
work_function(file_name)

file_name = "multivr2.py"
work_function(file_name)

file_name = "mv_zeros.py"
work_function(file_name)

file_name = "mvarpack.py"
work_function(file_name)

file_name = "mvexplct.py"
work_function(file_name)

file_name = "mvinter.py"
work_function(file_name)

file_name = "nc_tpath.py"
work_function(file_name)

file_name = "nc5axis.py"
work_function(file_name)

file_name = "ofstmtch.py"
work_function(file_name)

file_name = "orthotmc.py"
work_function(file_name)

file_name = "platonic.py"
work_function(file_name)

file_name = "playgrnd.py"
work_function(file_name)

file_name = "plotter.py"
work_function(file_name)

file_name = "plycrvtr.py"
work_function(file_name)

file_name = "pmorph.py"
work_function(file_name)

file_name = "polygons.py"
work_function(file_name)

file_name = "polypris.py"
work_function(file_name)

file_name = "polyprop.py"
work_function(file_name)

file_name = "polytrnc.py"
work_function(file_name)

file_name = "pp_apprx.py"
work_function(file_name)

file_name = "prim_fit.py"
work_function(file_name)

file_name = "primitiv.py"
work_function(file_name)

file_name = "primsrfs.py"
work_function(file_name)

file_name = "print_it.py"
work_function(file_name)

file_name = "printf.py"
work_function(file_name)

file_name = "prisa.py"
work_function(file_name)

file_name = "prisanim.py"
work_function(file_name)

file_name = "prismovi.py"
work_function(file_name)

file_name = "pt_incld.py"
work_function(file_name)

file_name = "pulleys.py"
work_function(file_name)

file_name = "puz3cube.py"
work_function(file_name)

file_name = "puz4pcs.py"
work_function(file_name)

file_name = "puz12pcs.py"
work_function(file_name)

file_name = "puz_anim.py"
work_function(file_name)

file_name = "puz_crnr.py"
work_function(file_name)

file_name = "puz_cube.py"
work_function(file_name)

file_name = "puz_dvd.py"
work_function(file_name)

file_name = "puz_snak.py"
work_function(file_name)

file_name = "puzcubes.py"
work_function(file_name)

file_name = "puzzles.py"
work_function(file_name)

file_name = "quadric.py"
work_function(file_name)

file_name = "ray_trap.py"
work_function(file_name)

file_name = "rbt_hand.py"
work_function(file_name)

file_name = "register.py"
work_function(file_name)

file_name = "rflct_ln.py"
work_function(file_name)

file_name = "ringring.py"
work_function(file_name)

file_name = "rrinter.py"
work_function(file_name)

file_name = "rvrs_eng.py"
work_function(file_name)

file_name = "saccess.py"
work_function(file_name)

file_name = "saloon2.py"
work_function(file_name)

file_name = "sbisect2.py"
work_function(file_name)

file_name = "sbisect.py"
work_function(file_name)

file_name = "selfintr.py"
work_function(file_name)

file_name = "silh.py"
work_function(file_name)

file_name = "skel2d.py"
work_function(file_name)

file_name = "smorph.py"
work_function(file_name)

file_name = "smorph2.py"
work_function(file_name)

file_name = "snakanim.py"
work_function(file_name)

file_name = "solid0.py"
work_function(file_name)

file_name = "solid1.py"
work_function(file_name)

file_name = "solid2.py"
work_function(file_name)

file_name = "solid2h.py"
work_function(file_name)

file_name = "solid3.py"
work_function(file_name)

file_name = "solid3h.py"
work_function(file_name)

file_name = "solid4.py"
work_function(file_name)

file_name = "solid4h.py"
work_function(file_name)

file_name = "solid5.py"
work_function(file_name)

file_name = "solid6.py"
work_function(file_name)

file_name = "solid6h.py"
work_function(file_name)

file_name = "solid7.py"
work_function(file_name)

file_name = "solid7h.py"
work_function(file_name)

file_name = "solid8.py"
work_function(file_name)

file_name = "solid8h.py"
work_function(file_name)

file_name = "solid9.py"
work_function(file_name)

file_name = "sphercon.py"
work_function(file_name)

file_name = "sqriacle.py"
work_function(file_name)

file_name = "srf_dist.py"
work_function(file_name)

file_name = "srf_krnl.py"
work_function(file_name)

file_name = "srf_ssi.py"
work_function(file_name)

file_name = "srf_tan.py"
work_function(file_name)

file_name = "srfray.py"
work_function(file_name)

file_name = "ssi-test.py"
work_function(file_name)

file_name = "surfrev.py"
work_function(file_name)

file_name = "symbolic.py"
work_function(file_name)

file_name = "tea-bool.py"
work_function(file_name)

file_name = "teacrvtr.py"
work_function(file_name)

file_name = "teapot.py"
work_function(file_name)

file_name = "teapot2.py"
work_function(file_name)

file_name = "techlogo.py"
work_function(file_name)

file_name = "textgeom.py"
work_function(file_name)

file_name = "textwarp.py"
work_function(file_name)

file_name = "tmorph.py"
work_function(file_name)

file_name = "trees.py"
work_function(file_name)

file_name = "triang.py"
work_function(file_name)

file_name = "trim_off.py"
work_function(file_name)

file_name = "trimsrfs.py"
work_function(file_name)

file_name = "trisrfs.py"
work_function(file_name)

file_name = "trivars.py"
work_function(file_name)

file_name = "turbine.py"
work_function(file_name)

file_name = "tvcover.py"
work_function(file_name)

file_name = "vor_cell.py"
work_function(file_name)

file_name = "warp_tea.py"
work_function(file_name)

file_name = "warp2trv.py"
work_function(file_name)

file_name = "warptriv.py"
work_function(file_name)

file_name = "weights.py"
work_function(file_name)

file_name = "wheel.py"
work_function(file_name)



