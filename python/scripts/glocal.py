#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  A simple example of curve morphing.
# 
#                                        Gershon Elber, March 1996.
# 

# 
#  Sets the viewing direction on the display device.
# 
save_mat = irit.GetViewMatrix()
irit.SetViewMatrix(  irit.roty( 180 ))
irit.viewobj( irit.GetViewMatrix() )
irit.SetViewMatrix(  save_mat)

irit.viewstate( "polyaprx", 1 )
irit.viewstate( "widthlines", 1 )

# 
#  Animation speed. The lower this number, the faster the animations will be,
#  skipping more frames.
# 
speed = 0.25

echosrc = irit.iritstate( "echosource", irit.GenIntObject(0 ))

# ###########################################################################
locally = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, 0.62705, (-0.418086 ) ), \
                                       irit.ctlpt( irit.E2, 0.593439, (-0.416962 ) ), \
                                       irit.ctlpt( irit.E2, 0.414706, (-0.366445 ) ), \
                                       irit.ctlpt( irit.E2, 0.362948, (-0.332803 ) ), \
                                       irit.ctlpt( irit.E2, 0.33727, (-0.293801 ) ), \
                                       irit.ctlpt( irit.E2, 0.288731, (-0.146024 ) ), \
                                       irit.ctlpt( irit.E2, 0.292251, (-0.0630604 ) ), \
                                       irit.ctlpt( irit.E2, 0.326431, (-0.0942595 ) ), \
                                       irit.ctlpt( irit.E2, 0.379121, (-0.513736 ) ), \
                                       irit.ctlpt( irit.E2, 0.324176, (-0.348901 ) ), \
                                       irit.ctlpt( irit.E2, 0.247253, (-0.337912 ) ), \
                                       irit.ctlpt( irit.E2, 0.285714, (-0.32967 ) ), \
                                       irit.ctlpt( irit.E2, 0.339394, (-0.378232 ) ), \
                                       irit.ctlpt( irit.E2, 0.285714, (-0.46978 ) ), \
                                       irit.ctlpt( irit.E2, 0.214286, (-0.458791 ) ), \
                                       irit.ctlpt( irit.E2, 0.200549, (-0.368132 ) ), \
                                       irit.ctlpt( irit.E2, 0.241758, (-0.337912 ) ), \
                                       irit.ctlpt( irit.E2, 0.145604, (-0.346154 ) ), \
                                       irit.ctlpt( irit.E2, 0.0714286, (-0.326923 ) ), \
                                       irit.ctlpt( irit.E2, 0.151099, (-0.348901 ) ), \
                                       irit.ctlpt( irit.E2, 0.175711, (-0.360335 ) ), \
                                       irit.ctlpt( irit.E2, 0.159341, (-0.447802 ) ), \
                                       irit.ctlpt( irit.E2, 0.0659341, (-0.425824 ) ), \
                                       irit.ctlpt( irit.E2, 0.0192308, (-0.365385 ) ), \
                                       irit.ctlpt( irit.E2, (-0.0631868 ), (-0.348901 ) ), \
                                       irit.ctlpt( irit.E2, (-0.0604396 ), (-0.351648 ) ), \
                                       irit.ctlpt( irit.E2, (-0.0147518 ), (-0.367252 ) ), \
                                       irit.ctlpt( irit.E2, 0.0202549, (-0.396131 ) ), \
                                       irit.ctlpt( irit.E2, 0.0306359, (-0.41967 ) ), \
                                       irit.ctlpt( irit.E2, 0.00413766, (-0.447826 ) ), \
                                       irit.ctlpt( irit.E2, (-0.0567533 ), (-0.42663 ) ), \
                                       irit.ctlpt( irit.E2, (-0.0635566 ), (-0.39429 ) ), \
                                       irit.ctlpt( irit.E2, (-0.0686813 ), (-0.357143 ) ), \
                                       irit.ctlpt( irit.E2, (-0.0575531 ), (-0.346678 ) ), \
                                       irit.ctlpt( irit.E2, (-0.0464482 ), (-0.397592 ) ), \
                                       irit.ctlpt( irit.E2, (-0.0510823 ), (-0.431478 ) ), \
                                       irit.ctlpt( irit.E2, (-0.0697253 ), (-0.455784 ) ), \
                                       irit.ctlpt( irit.E2, (-0.122527 ), (-0.441885 ) ), \
                                       irit.ctlpt( irit.E2, (-0.161539 ), (-0.384224 ) ), \
                                       irit.ctlpt( irit.E2, (-0.18956 ), (-0.302198 ) ), \
                                       irit.ctlpt( irit.E2, (-0.247253 ), (-0.0659341 ) ), \
                                       irit.ctlpt( irit.E2, (-0.145604 ), (-0.332418 ) ), \
                                       irit.ctlpt( irit.E2, (-0.14011 ), (-0.445055 ) ), \
                                       irit.ctlpt( irit.E2, (-0.255495 ), (-0.445055 ) ), \
                                       irit.ctlpt( irit.E2, (-0.302198 ), (-0.307692 ) ), \
                                       irit.ctlpt( irit.E2, (-0.35749 ), (-0.0587375 ) ), \
                                       irit.ctlpt( irit.E2, (-0.269231 ), (-0.285714 ) ), \
                                       irit.ctlpt( irit.E2, (-0.299451 ), (-0.453297 ) ), \
                                       irit.ctlpt( irit.E2, (-0.377342 ), (-0.390006 ) ), \
                                       irit.ctlpt( irit.E2, (-0.391946 ), (-0.273712 ) ), \
                                       irit.ctlpt( irit.E2, (-0.384193 ), (-0.298763 ) ), \
                                       irit.ctlpt( irit.E2, (-0.370793 ), (-0.373895 ) ), \
                                       irit.ctlpt( irit.E2, (-0.367854 ), (-0.402319 ) ), \
                                       irit.ctlpt( irit.E2, (-0.42033 ), (-0.425824 ) ), \
                                       irit.ctlpt( irit.E2, (-0.471097 ), (-0.323993 ) ), \
                                       irit.ctlpt( irit.E2, (-0.468744 ), (-0.312264 ) ), \
                                       irit.ctlpt( irit.E2, (-0.471475 ), (-0.382282 ) ), \
                                       irit.ctlpt( irit.E2, (-0.453039 ), (-0.555819 ) ), \
                                       irit.ctlpt( irit.E2, (-0.337647 ), (-0.586488 ) ), \
                                       irit.ctlpt( irit.E2, (-0.352405 ), (-0.544612 ) ), \
                                       irit.ctlpt( irit.E2, (-0.467128 ), (-0.523359 ) ) ), irit.list( 0, 0, 0, 0.0169492, 0.0338983, 0.0508475,\
0.0677966, 0.0847458, 0.101695, 0.118644, 0.135593, 0.152542,\
0.169492, 0.186441, 0.20339, 0.220339, 0.237288, 0.254237,\
0.271186, 0.288136, 0.305085, 0.322034, 0.338983, 0.355932,\
0.372881, 0.389831, 0.40678, 0.423729, 0.440678, 0.457627,\
0.474576, 0.491525, 0.508475, 0.525424, 0.542373, 0.559322,\
0.576271, 0.59322, 0.610169, 0.627119, 0.644068, 0.661017,\
0.677966, 0.694915, 0.711864, 0.728814, 0.745763, 0.762712,\
0.779661, 0.79661, 0.813559, 0.830508, 0.847458, 0.864407,\
0.881356, 0.898305, 0.915254, 0.932203, 0.949153, 0.966102,\
0.983051, 1, 1, 1 ) )

globally = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, 0.409341, 0.456044 ), \
                                        irit.ctlpt( irit.E2, 0.495896, 0.463099 ), \
                                        irit.ctlpt( irit.E2, 0.526756, 0.459253 ), \
                                        irit.ctlpt( irit.E2, 0.56319, 0.436464 ), \
                                        irit.ctlpt( irit.E2, 0.575311, 0.37219 ), \
                                        irit.ctlpt( irit.E2, 0.443829, 0.352561 ), \
                                        irit.ctlpt( irit.E2, 0.381178, 0.375885 ), \
                                        irit.ctlpt( irit.E2, 0.39083, 0.442424 ), \
                                        irit.ctlpt( irit.E2, 0.393845, 0.452091 ), \
                                        irit.ctlpt( irit.E2, 0.392158, 0.456488 ), \
                                        irit.ctlpt( irit.E2, 0.393817, 0.377047 ), \
                                        irit.ctlpt( irit.E2, 0.423314, 0.207375 ), \
                                        irit.ctlpt( irit.E2, 0.535714, 0.0604396 ), \
                                        irit.ctlpt( irit.E2, 0.424317, 0.313974 ), \
                                        irit.ctlpt( irit.E2, 0.300315, 0.373214 ), \
                                        irit.ctlpt( irit.E2, 0.217076, 0.572057 ), \
                                        irit.ctlpt( irit.E2, 0.204434, 0.692412 ), \
                                        irit.ctlpt( irit.E2, 0.202976, 0.705647 ), \
                                        irit.ctlpt( irit.E2, 0.223419, 0.662085 ), \
                                        irit.ctlpt( irit.E2, 0.264346, 0.543445 ), \
                                        irit.ctlpt( irit.E2, 0.272216, 0.438565 ), \
                                        irit.ctlpt( irit.E2, 0.279823, 0.392373 ), \
                                        irit.ctlpt( irit.E2, 0.252354, 0.386253 ), \
                                        irit.ctlpt( irit.E2, 0.181319, 0.436813 ), \
                                        irit.ctlpt( irit.E2, 0.199259, 0.400732 ), \
                                        irit.ctlpt( irit.E2, 0.213134, 0.347925 ), \
                                        irit.ctlpt( irit.E2, 0.152955, 0.36674 ), \
                                        irit.ctlpt( irit.E2, 0.135194, 0.394466 ), \
                                        irit.ctlpt( irit.E2, 0.143426, 0.433858 ), \
                                        irit.ctlpt( irit.E2, 0.155053, 0.437899 ), \
                                        irit.ctlpt( irit.E2, 0.171634, 0.435626 ), \
                                        irit.ctlpt( irit.E2, 0.185458, 0.433952 ), \
                                        irit.ctlpt( irit.E2, 0.0495181, 0.432913 ), \
                                        irit.ctlpt( irit.E2, 0.0170595, 0.440174 ), \
                                        irit.ctlpt( irit.E2, 0.0190177, 0.449884 ), \
                                        irit.ctlpt( irit.E2, (-0.0230322 ), 0.650896 ), \
                                        irit.ctlpt( irit.E2, 0, 0.607143 ), \
                                        irit.ctlpt( irit.E2, 0.0108935, 0.436632 ), \
                                        irit.ctlpt( irit.E2, 0.0349947, 0.30953 ), \
                                        irit.ctlpt( irit.E2, 0.0423563, 0.38203 ), \
                                        irit.ctlpt( irit.E2, 0.00549451, 0.445055 ), \
                                        irit.ctlpt( irit.E2, (-0.0686813 ), 0.442308 ), \
                                        irit.ctlpt( irit.E2, (-0.0479742 ), 0.371294 ), \
                                        irit.ctlpt( irit.E2, 0.0174776, 0.354734 ), \
                                        irit.ctlpt( irit.E2, 0.0412115, 0.367527 ), \
                                        irit.ctlpt( irit.E2, (-0.134615 ), 0.412088 ), \
                                        irit.ctlpt( irit.E2, (-0.214663 ), 0.411028 ), \
                                        irit.ctlpt( irit.E2, (-0.192762 ), 0.422809 ), \
                                        irit.ctlpt( irit.E2, (-0.104396 ), 0.395604 ), \
                                        irit.ctlpt( irit.E2, (-0.097058 ), 0.335892 ), \
                                        irit.ctlpt( irit.E2, (-0.203297 ), 0.335165 ), \
                                        irit.ctlpt( irit.E2, (-0.195055 ), 0.42033 ), \
                                        irit.ctlpt( irit.E2, (-0.17033 ), 0.296703 ), \
                                        irit.ctlpt( irit.E2, (-0.205187 ), 0.314129 ), \
                                        irit.ctlpt( irit.E2, (-0.32967 ), 0.478022 ), \
                                        irit.ctlpt( irit.E2, (-0.349878 ), 0.693383 ), \
                                        irit.ctlpt( irit.E2, (-0.273502 ), 0.525542 ), \
                                        irit.ctlpt( irit.E2, (-0.266794 ), 0.324805 ), \
                                        irit.ctlpt( irit.E2, (-0.29383 ), 0.326754 ), \
                                        irit.ctlpt( irit.E2, (-0.453297 ), 0.5 ), \
                                        irit.ctlpt( irit.E2, (-0.456044 ), 0.692308 ), \
                                        irit.ctlpt( irit.E2, (-0.392857 ), 0.527473 ), \
                                        irit.ctlpt( irit.E2, (-0.417582 ), 0.293956 ), \
                                        irit.ctlpt( irit.E2, (-0.480769 ), 0.302198 ), \
                                        irit.ctlpt( irit.E2, (-0.508242 ), 0.442308 ), \
                                        irit.ctlpt( irit.E2, (-0.516484 ), 0.340659 ), \
                                        irit.ctlpt( irit.E2, (-0.571429 ), 0.32967 ), \
                                        irit.ctlpt( irit.E2, (-0.601648 ), 0.431319 ), \
                                        irit.ctlpt( irit.E2, (-0.596154 ), 0.271978 ), \
                                        irit.ctlpt( irit.E2, (-0.535714 ), 0.181319 ), \
                                        irit.ctlpt( irit.E2, (-0.461538 ), 0.173077 ) ), irit.list( 0, 0, 0, 0.0144928, 0.0289855, 0.0434783,\
0.057971, 0.0724638, 0.0869565, 0.101449, 0.115942, 0.130435,\
0.144928, 0.15942, 0.173913, 0.188406, 0.202899, 0.217391,\
0.231884, 0.246377, 0.26087, 0.275362, 0.289855, 0.304348,\
0.318841, 0.333333, 0.347826, 0.362319, 0.376812, 0.391304,\
0.405797, 0.42029, 0.434783, 0.449275, 0.463768, 0.478261,\
0.492754, 0.507246, 0.521739, 0.536232, 0.550725, 0.565217,\
0.57971, 0.594203, 0.608696, 0.623188, 0.637681, 0.652174,\
0.666667, 0.681159, 0.695652, 0.710145, 0.724638, 0.73913,\
0.753623, 0.768116, 0.782609, 0.797101, 0.811594, 0.826087,\
0.84058, 0.855072, 0.869565, 0.884058, 0.898551, 0.913043,\
0.927536, 0.942029, 0.956522, 0.971014, 0.985507, 1,\
1, 1 ) )

echosrc = irit.iritstate( "echosource", echosrc )
irit.free( echosrc )

irit.ffcompat( locally, globally )

irit.color( locally, irit.RED )
irit.color( globally, irit.GREEN )

irit.view( irit.list( locally, globally ), irit.ON )
bg_obj = irit.list( locally, globally )
i = 0
while ( i <= 300 * speed ):
    c = irit.cmorph( locally, globally, 0, i/(300.0 * speed) )
    irit.color( c, irit.YELLOW )
    irit.view( irit.list(c, bg_obj), irit.ON)
    i = i + 1
irit.save( "glocal1", irit.list( locally, globally, irit.cmorph( locally, globally, 0, 0.5 ) ) )

globally2 = irit.ffmatch( locally, globally, 20, 100, 2, 0, (-2 ) )
irit.ffcompat( locally, globally2 )
i = 0
while ( i <= 100 * speed ):
    c = irit.cmorph( locally, globally2, 0, i/(100.0 * speed) )
    irit.color( c, irit.YELLOW )
    irit.view( irit.list(c, bg_obj), irit.ON)
    i = i + 1
irit.save( "glocal2", irit.list( locally, globally, irit.cmorph( locally, globally2, 0, 0.5 ) ) )

irit.viewstate( "polyaprx", 0 )
irit.viewstate( "widthlines", 0 )

irit.free( locally )
irit.free( globally )
irit.free( globally2 )
irit.free( c )

