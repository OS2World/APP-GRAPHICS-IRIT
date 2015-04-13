#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  The infamous Teapot data as four B-spline surfaces.  Herein, we compute
#  the Gaussian, Mean, and Curvature bounds as scalar fields attached to the
#  geometry.  Saved data can be rendered and visualized using 'irender'.
# 

echosrc = irit.iritstate( "echosource", irit.GenRealObject(0) )

body = irit.sbspline( 4, 4, irit.list( irit.list( irit.ctlpt( irit.E3, 1.4, 2.25, 0 ), \
                                                  irit.ctlpt( irit.E3, 1.3375, 2.38125, 0 ), \
                                                  irit.ctlpt( irit.E3, 1.4375, 2.38125, 0 ), \
                                                  irit.ctlpt( irit.E3, 1.5, 2.25, 0 ), \
                                                  irit.ctlpt( irit.E3, 1.75, 1.725, 0 ), \
                                                  irit.ctlpt( irit.E3, 2, 1.2, 0 ), \
                                                  irit.ctlpt( irit.E3, 2, 0.75, 0 ), \
                                                  irit.ctlpt( irit.E3, 2, 0.3, 0 ), \
                                                  irit.ctlpt( irit.E3, 1.5, 0.075, 0 ), \
                                                  irit.ctlpt( irit.E3, 1.5, 0, 0 ) ), irit.list( \
                                                  irit.ctlpt( irit.E3, 1.4, 2.25, 0.784 ), \
                                                  irit.ctlpt( irit.E3, 1.3375, 2.38125, 0.749 ), \
                                                  irit.ctlpt( irit.E3, 1.4375, 2.38125, 0.805 ), \
                                                  irit.ctlpt( irit.E3, 1.5, 2.25, 0.84 ), \
                                                  irit.ctlpt( irit.E3, 1.75, 1.725, 0.98 ), \
                                                  irit.ctlpt( irit.E3, 2, 1.2, 1.12 ), \
                                                  irit.ctlpt( irit.E3, 2, 0.75, 1.12 ), \
                                                  irit.ctlpt( irit.E3, 2, 0.3, 1.12 ), \
                                                  irit.ctlpt( irit.E3, 1.5, 0.075, 0.84 ), \
                                                  irit.ctlpt( irit.E3, 1.5, 0, 0.84 ) ), irit.list( \
                                                  irit.ctlpt( irit.E3, 0.784, 2.25, 1.4 ), \
                                                  irit.ctlpt( irit.E3, 0.749, 2.38125, 1.3375 ), \
                                                  irit.ctlpt( irit.E3, 0.805, 2.38125, 1.4375 ), \
                                                  irit.ctlpt( irit.E3, 0.84, 2.25, 1.5 ), \
                                                  irit.ctlpt( irit.E3, 0.98, 1.725, 1.75 ), \
                                                  irit.ctlpt( irit.E3, 1.12, 1.2, 2 ), \
                                                  irit.ctlpt( irit.E3, 1.12, 0.75, 2 ), \
                                                  irit.ctlpt( irit.E3, 1.12, 0.3, 2 ), \
                                                  irit.ctlpt( irit.E3, 0.84, 0.075, 1.5 ), \
                                                  irit.ctlpt( irit.E3, 0.84, 0, 1.5 ) ), irit.list( \
                                                  irit.ctlpt( irit.E3, 0, 2.25, 1.4 ), \
                                                  irit.ctlpt( irit.E3, 0, 2.38125, 1.3375 ), \
                                                  irit.ctlpt( irit.E3, 0, 2.38125, 1.4375 ), \
                                                  irit.ctlpt( irit.E3, 0, 2.25, 1.5 ), \
                                                  irit.ctlpt( irit.E3, 0, 1.725, 1.75 ), \
                                                  irit.ctlpt( irit.E3, 0, 1.2, 2 ), \
                                                  irit.ctlpt( irit.E3, 0, 0.75, 2 ), \
                                                  irit.ctlpt( irit.E3, 0, 0.3, 2 ), \
                                                  irit.ctlpt( irit.E3, 0, 0.075, 1.5 ), \
                                                  irit.ctlpt( irit.E3, 0, 0, 1.5 ) ), irit.list( \
                                                  irit.ctlpt( irit.E3, (-0.784 ), 2.25, 1.4 ), \
                                                  irit.ctlpt( irit.E3, (-0.749 ), 2.38125, 1.3375 ), \
                                                  irit.ctlpt( irit.E3, (-0.805 ), 2.38125, 1.4375 ), \
                                                  irit.ctlpt( irit.E3, (-0.84 ), 2.25, 1.5 ), \
                                                  irit.ctlpt( irit.E3, (-0.98 ), 1.725, 1.75 ), \
                                                  irit.ctlpt( irit.E3, (-1.12 ), 1.2, 2 ), \
                                                  irit.ctlpt( irit.E3, (-1.12 ), 0.75, 2 ), \
                                                  irit.ctlpt( irit.E3, (-1.12 ), 0.3, 2 ), \
                                                  irit.ctlpt( irit.E3, (-0.84 ), 0.075, 1.5 ), \
                                                  irit.ctlpt( irit.E3, (-0.84 ), 0, 1.5 ) ), irit.list( \
                                                  irit.ctlpt( irit.E3, (-1.4 ), 2.25, 0.784 ), \
                                                  irit.ctlpt( irit.E3, (-1.3375 ), 2.38125, 0.749 ), \
                                                  irit.ctlpt( irit.E3, (-1.4375 ), 2.38125, 0.805 ), \
                                                  irit.ctlpt( irit.E3, (-1.5 ), 2.25, 0.84 ), \
                                                  irit.ctlpt( irit.E3, (-1.75 ), 1.725, 0.98 ), \
                                                  irit.ctlpt( irit.E3, (-2 ), 1.2, 1.12 ), \
                                                  irit.ctlpt( irit.E3, (-2 ), 0.75, 1.12 ), \
                                                  irit.ctlpt( irit.E3, (-2 ), 0.3, 1.12 ), \
                                                  irit.ctlpt( irit.E3, (-1.5 ), 0.075, 0.84 ), \
                                                  irit.ctlpt( irit.E3, (-1.5 ), 0, 0.84 ) ), irit.list( \
                                                  irit.ctlpt( irit.E3, (-1.4 ), 2.25, 0 ), \
                                                  irit.ctlpt( irit.E3, (-1.3375 ), 2.38125, 0 ), \
                                                  irit.ctlpt( irit.E3, (-1.4375 ), 2.38125, 0 ), \
                                                  irit.ctlpt( irit.E3, (-1.5 ), 2.25, 0 ), \
                                                  irit.ctlpt( irit.E3, (-1.75 ), 1.725, 0 ), \
                                                  irit.ctlpt( irit.E3, (-2 ), 1.2, 0 ), \
                                                  irit.ctlpt( irit.E3, (-2 ), 0.75, 0 ), \
                                                  irit.ctlpt( irit.E3, (-2 ), 0.3, 0 ), \
                                                  irit.ctlpt( irit.E3, (-1.5 ), 0.075, 0 ), \
                                                  irit.ctlpt( irit.E3, (-1.5 ), 0, 0 ) ), irit.list( \
                                                  irit.ctlpt( irit.E3, (-1.4 ), 2.25, (-0.784 ) ), \
                                                  irit.ctlpt( irit.E3, (-1.3375 ), 2.38125, (-0.749 ) ), \
                                                  irit.ctlpt( irit.E3, (-1.4375 ), 2.38125, (-0.805 ) ), \
                                                  irit.ctlpt( irit.E3, (-1.5 ), 2.25, (-0.84 ) ), \
                                                  irit.ctlpt( irit.E3, (-1.75 ), 1.725, (-0.98 ) ), \
                                                  irit.ctlpt( irit.E3, (-2 ), 1.2, (-1.12 ) ), \
                                                  irit.ctlpt( irit.E3, (-2 ), 0.75, (-1.12 ) ), \
                                                  irit.ctlpt( irit.E3, (-2 ), 0.3, (-1.12 ) ), \
                                                  irit.ctlpt( irit.E3, (-1.5 ), 0.075, (-0.84 ) ), \
                                                  irit.ctlpt( irit.E3, (-1.5 ), 0, (-0.84 ) ) ), irit.list( \
                                                  irit.ctlpt( irit.E3, (-0.784 ), 2.25, (-1.4 ) ), \
                                                  irit.ctlpt( irit.E3, (-0.749 ), 2.38125, (-1.3375 ) ), \
                                                  irit.ctlpt( irit.E3, (-0.805 ), 2.38125, (-1.4375 ) ), \
                                                  irit.ctlpt( irit.E3, (-0.84 ), 2.25, (-1.5 ) ), \
                                                  irit.ctlpt( irit.E3, (-0.98 ), 1.725, (-1.75 ) ), \
                                                  irit.ctlpt( irit.E3, (-1.12 ), 1.2, (-2 ) ), \
                                                  irit.ctlpt( irit.E3, (-1.12 ), 0.75, (-2 ) ), \
                                                  irit.ctlpt( irit.E3, (-1.12 ), 0.3, (-2 ) ), \
                                                  irit.ctlpt( irit.E3, (-0.84 ), 0.075, (-1.5 ) ), \
                                                  irit.ctlpt( irit.E3, (-0.84 ), 0, (-1.5 ) ) ), irit.list( \
                                                  irit.ctlpt( irit.E3, 0, 2.25, (-1.4 ) ), \
                                                  irit.ctlpt( irit.E3, 0, 2.38125, (-1.3375 ) ), \
                                                  irit.ctlpt( irit.E3, 0, 2.38125, (-1.4375 ) ), \
                                                  irit.ctlpt( irit.E3, 0, 2.25, (-1.5 ) ), \
                                                  irit.ctlpt( irit.E3, 0, 1.725, (-1.75 ) ), \
                                                  irit.ctlpt( irit.E3, 0, 1.2, (-2 ) ), \
                                                  irit.ctlpt( irit.E3, 0, 0.75, (-2 ) ), \
                                                  irit.ctlpt( irit.E3, 0, 0.3, (-2 ) ), \
                                                  irit.ctlpt( irit.E3, 0, 0.075, (-1.5 ) ), \
                                                  irit.ctlpt( irit.E3, 0, 0, (-1.5 ) ) ), irit.list( \
                                                  irit.ctlpt( irit.E3, 0.784, 2.25, (-1.4 ) ), \
                                                  irit.ctlpt( irit.E3, 0.749, 2.38125, (-1.3375 ) ), \
                                                  irit.ctlpt( irit.E3, 0.805, 2.38125, (-1.4375 ) ), \
                                                  irit.ctlpt( irit.E3, 0.84, 2.25, (-1.5 ) ), \
                                                  irit.ctlpt( irit.E3, 0.98, 1.725, (-1.75 ) ), \
                                                  irit.ctlpt( irit.E3, 1.12, 1.2, (-2 ) ), \
                                                  irit.ctlpt( irit.E3, 1.12, 0.75, (-2 ) ), \
                                                  irit.ctlpt( irit.E3, 1.12, 0.3, (-2 ) ), \
                                                  irit.ctlpt( irit.E3, 0.84, 0.075, (-1.5 ) ), \
                                                  irit.ctlpt( irit.E3, 0.84, 0, (-1.5 ) ) ), irit.list( \
                                                  irit.ctlpt( irit.E3, 1.4, 2.25, (-0.784 ) ), \
                                                  irit.ctlpt( irit.E3, 1.3375, 2.38125, (-0.749 ) ), \
                                                  irit.ctlpt( irit.E3, 1.4375, 2.38125, (-0.805 ) ), \
                                                  irit.ctlpt( irit.E3, 1.5, 2.25, (-0.84 ) ), \
                                                  irit.ctlpt( irit.E3, 1.75, 1.725, (-0.98 ) ), \
                                                  irit.ctlpt( irit.E3, 2, 1.2, (-1.12 ) ), \
                                                  irit.ctlpt( irit.E3, 2, 0.75, (-1.12 ) ), \
                                                  irit.ctlpt( irit.E3, 2, 0.3, (-1.12 ) ), \
                                                  irit.ctlpt( irit.E3, 1.5, 0.075, (-0.84 ) ), \
                                                  irit.ctlpt( irit.E3, 1.5, 0, (-0.84 ) ) ), irit.list( \
                                                  irit.ctlpt( irit.E3, 1.4, 2.25, 0 ), \
                                                  irit.ctlpt( irit.E3, 1.3375, 2.38125, 0 ), \
                                                  irit.ctlpt( irit.E3, 1.4375, 2.38125, 0 ), \
                                                  irit.ctlpt( irit.E3, 1.5, 2.25, 0 ), \
                                                  irit.ctlpt( irit.E3, 1.75, 1.725, 0 ), \
                                                  irit.ctlpt( irit.E3, 2, 1.2, 0 ), \
                                                  irit.ctlpt( irit.E3, 2, 0.75, 0 ), \
                                                  irit.ctlpt( irit.E3, 2, 0.3, 0 ), \
                                                  irit.ctlpt( irit.E3, 1.5, 0.075, 0 ), \
                                                  irit.ctlpt( irit.E3, 1.5, 0, 0 ) ) ), irit.list( irit.list( 0, 0, 0, 0, 1, 1,\
1, 2, 2, 2, 3, 3,\
3, 3 ), irit.list( 0, 0, 0, 0, 1, 1,\
1, 2, 2, 2, 3, 3,\
3, 4, 4, 4, 4 ) ) )
spout = irit.sbspline( 4, 4, irit.list( irit.list( irit.ctlpt( irit.E3, 1.7, 1.275, 0 ), \
                                                   irit.ctlpt( irit.E3, 2.6, 1.275, 0 ), \
                                                   irit.ctlpt( irit.E3, 2.3, 1.95, 0 ), \
                                                   irit.ctlpt( irit.E3, 2.7, 2.25, 0 ), \
                                                   irit.ctlpt( irit.E3, 2.8, 2.325, 0 ), \
                                                   irit.ctlpt( irit.E3, 2.9, 2.325, 0 ), \
                                                   irit.ctlpt( irit.E3, 2.8, 2.25, 0 ) ), irit.list( \
                                                   irit.ctlpt( irit.E3, 1.7, 1.275, 0.66 ), \
                                                   irit.ctlpt( irit.E3, 2.6, 1.275, 0.66 ), \
                                                   irit.ctlpt( irit.E3, 2.3, 1.95, 0.25 ), \
                                                   irit.ctlpt( irit.E3, 2.7, 2.25, 0.25 ), \
                                                   irit.ctlpt( irit.E3, 2.8, 2.325, 0.25 ), \
                                                   irit.ctlpt( irit.E3, 2.9, 2.325, 0.15 ), \
                                                   irit.ctlpt( irit.E3, 2.8, 2.25, 0.15 ) ), irit.list( \
                                                   irit.ctlpt( irit.E3, 1.7, 0.45, 0.66 ), \
                                                   irit.ctlpt( irit.E3, 3.1, 0.675, 0.66 ), \
                                                   irit.ctlpt( irit.E3, 2.4, 1.875, 0.25 ), \
                                                   irit.ctlpt( irit.E3, 3.3, 2.25, 0.25 ), \
                                                   irit.ctlpt( irit.E3, 3.525, 2.34375, 0.25 ), \
                                                   irit.ctlpt( irit.E3, 3.45, 2.3625, 0.15 ), \
                                                   irit.ctlpt( irit.E3, 3.2, 2.25, 0.15 ) ), irit.list( \
                                                   irit.ctlpt( irit.E3, 1.7, 0.45, 0 ), \
                                                   irit.ctlpt( irit.E3, 3.1, 0.675, 0 ), \
                                                   irit.ctlpt( irit.E3, 2.4, 1.875, 0 ), \
                                                   irit.ctlpt( irit.E3, 3.3, 2.25, 0 ), \
                                                   irit.ctlpt( irit.E3, 3.525, 2.34375, 0 ), \
                                                   irit.ctlpt( irit.E3, 3.45, 2.3625, 0 ), \
                                                   irit.ctlpt( irit.E3, 3.2, 2.25, 0 ) ), irit.list( \
                                                   irit.ctlpt( irit.E3, 1.7, 0.45, (-0.66 ) ), \
                                                   irit.ctlpt( irit.E3, 3.1, 0.675, (-0.66 ) ), \
                                                   irit.ctlpt( irit.E3, 2.4, 1.875, (-0.25 ) ), \
                                                   irit.ctlpt( irit.E3, 3.3, 2.25, (-0.25 ) ), \
                                                   irit.ctlpt( irit.E3, 3.525, 2.34375, (-0.25 ) ), \
                                                   irit.ctlpt( irit.E3, 3.45, 2.3625, (-0.15 ) ), \
                                                   irit.ctlpt( irit.E3, 3.2, 2.25, (-0.15 ) ) ), irit.list( \
                                                   irit.ctlpt( irit.E3, 1.7, 1.275, (-0.66 ) ), \
                                                   irit.ctlpt( irit.E3, 2.6, 1.275, (-0.66 ) ), \
                                                   irit.ctlpt( irit.E3, 2.3, 1.95, (-0.25 ) ), \
                                                   irit.ctlpt( irit.E3, 2.7, 2.25, (-0.25 ) ), \
                                                   irit.ctlpt( irit.E3, 2.8, 2.325, (-0.25 ) ), \
                                                   irit.ctlpt( irit.E3, 2.9, 2.325, (-0.15 ) ), \
                                                   irit.ctlpt( irit.E3, 2.8, 2.25, (-0.15 ) ) ), irit.list( \
                                                   irit.ctlpt( irit.E3, 1.7, 1.275, 0 ), \
                                                   irit.ctlpt( irit.E3, 2.6, 1.275, 0 ), \
                                                   irit.ctlpt( irit.E3, 2.3, 1.95, 0 ), \
                                                   irit.ctlpt( irit.E3, 2.7, 2.25, 0 ), \
                                                   irit.ctlpt( irit.E3, 2.8, 2.325, 0 ), \
                                                   irit.ctlpt( irit.E3, 2.9, 2.325, 0 ), \
                                                   irit.ctlpt( irit.E3, 2.8, 2.25, 0 ) ) ), irit.list( irit.list( 0, 0, 0, 0, 1, 1,\
1, 2, 2, 2, 2 ), irit.list( 0, 0, 0, 0, 1, 1,\
1, 2, 2, 2, 2 ) ) )
handle = irit.sbspline( 4, 4, irit.list( irit.list( irit.ctlpt( irit.E3, (-1.595 ), 1.875, 0 ), \
                                                    irit.ctlpt( irit.E3, (-2.295 ), 1.875, 0 ), \
                                                    irit.ctlpt( irit.E3, (-2.695 ), 1.875, 0 ), \
                                                    irit.ctlpt( irit.E3, (-2.695 ), 1.65, 0 ), \
                                                    irit.ctlpt( irit.E3, (-2.695 ), 1.425, 0 ), \
                                                    irit.ctlpt( irit.E3, (-2.495 ), 0.975, 0 ), \
                                                    irit.ctlpt( irit.E3, (-1.995 ), 0.75, 0 ) ), irit.list( \
                                                    irit.ctlpt( irit.E3, (-1.595 ), 1.875, 0.3 ), \
                                                    irit.ctlpt( irit.E3, (-2.295 ), 1.875, 0.3 ), \
                                                    irit.ctlpt( irit.E3, (-2.695 ), 1.875, 0.3 ), \
                                                    irit.ctlpt( irit.E3, (-2.695 ), 1.65, 0.3 ), \
                                                    irit.ctlpt( irit.E3, (-2.695 ), 1.425, 0.3 ), \
                                                    irit.ctlpt( irit.E3, (-2.495 ), 0.975, 0.3 ), \
                                                    irit.ctlpt( irit.E3, (-1.995 ), 0.75, 0.3 ) ), irit.list( \
                                                    irit.ctlpt( irit.E3, (-1.495 ), 2.1, 0.3 ), \
                                                    irit.ctlpt( irit.E3, (-2.495 ), 2.1, 0.3 ), \
                                                    irit.ctlpt( irit.E3, (-2.995 ), 2.1, 0.3 ), \
                                                    irit.ctlpt( irit.E3, (-2.995 ), 1.65, 0.3 ), \
                                                    irit.ctlpt( irit.E3, (-2.995 ), 1.2, 0.3 ), \
                                                    irit.ctlpt( irit.E3, (-2.645 ), 0.7875, 0.3 ), \
                                                    irit.ctlpt( irit.E3, (-1.895 ), 0.45, 0.3 ) ), irit.list( \
                                                    irit.ctlpt( irit.E3, (-1.495 ), 2.1, 0 ), \
                                                    irit.ctlpt( irit.E3, (-2.495 ), 2.1, 0 ), \
                                                    irit.ctlpt( irit.E3, (-2.995 ), 2.1, 0 ), \
                                                    irit.ctlpt( irit.E3, (-2.995 ), 1.65, 0 ), \
                                                    irit.ctlpt( irit.E3, (-2.995 ), 1.2, 0 ), \
                                                    irit.ctlpt( irit.E3, (-2.645 ), 0.7875, 0 ), \
                                                    irit.ctlpt( irit.E3, (-1.895 ), 0.45, 0 ) ), irit.list( \
                                                    irit.ctlpt( irit.E3, (-1.495 ), 2.1, (-0.3 ) ), \
                                                    irit.ctlpt( irit.E3, (-2.495 ), 2.1, (-0.3 ) ), \
                                                    irit.ctlpt( irit.E3, (-2.995 ), 2.1, (-0.3 ) ), \
                                                    irit.ctlpt( irit.E3, (-2.995 ), 1.65, (-0.3 ) ), \
                                                    irit.ctlpt( irit.E3, (-2.995 ), 1.2, (-0.3 ) ), \
                                                    irit.ctlpt( irit.E3, (-2.645 ), 0.7875, (-0.3 ) ), \
                                                    irit.ctlpt( irit.E3, (-1.895 ), 0.45, (-0.3 ) ) ), irit.list( \
                                                    irit.ctlpt( irit.E3, (-1.595 ), 1.875, (-0.3 ) ), \
                                                    irit.ctlpt( irit.E3, (-2.295 ), 1.875, (-0.3 ) ), \
                                                    irit.ctlpt( irit.E3, (-2.695 ), 1.875, (-0.3 ) ), \
                                                    irit.ctlpt( irit.E3, (-2.695 ), 1.65, (-0.3 ) ), \
                                                    irit.ctlpt( irit.E3, (-2.695 ), 1.425, (-0.3 ) ), \
                                                    irit.ctlpt( irit.E3, (-2.495 ), 0.975, (-0.3 ) ), \
                                                    irit.ctlpt( irit.E3, (-1.995 ), 0.75, (-0.3 ) ) ), irit.list( \
                                                    irit.ctlpt( irit.E3, (-1.595 ), 1.875, 0 ), \
                                                    irit.ctlpt( irit.E3, (-2.295 ), 1.875, 0 ), \
                                                    irit.ctlpt( irit.E3, (-2.695 ), 1.875, 0 ), \
                                                    irit.ctlpt( irit.E3, (-2.695 ), 1.65, 0 ), \
                                                    irit.ctlpt( irit.E3, (-2.695 ), 1.425, 0 ), \
                                                    irit.ctlpt( irit.E3, (-2.495 ), 0.975, 0 ), \
                                                    irit.ctlpt( irit.E3, (-1.995 ), 0.75, 0 ) ) ), irit.list( irit.list( 0, 0, 0, 0, 1, 1,\
1, 2, 2, 2, 2 ), irit.list( 0, 0, 0, 0, 1, 1,\
1, 2, 2, 2, 2 ) ) )
cap = irit.sbspline( 4, 4, irit.list( irit.list( irit.ctlpt( irit.E3, 0, 3, 0 ), \
                                                 irit.ctlpt( irit.E3, 0.8, 3, 0 ), \
                                                 irit.ctlpt( irit.E3, 0, 2.7, 0 ), \
                                                 irit.ctlpt( irit.E3, 0.2, 2.55, 0 ), \
                                                 irit.ctlpt( irit.E3, 0.4, 2.4, 0 ), \
                                                 irit.ctlpt( irit.E3, 1.3, 2.4, 0 ), \
                                                 irit.ctlpt( irit.E3, 1.3, 2.25, 0 ) ), irit.list( \
                                                 irit.ctlpt( irit.E3, 0, 3, 0.002 ), \
                                                 irit.ctlpt( irit.E3, 0.8, 3, 0.45 ), \
                                                 irit.ctlpt( irit.E3, 0, 2.7, 0 ), \
                                                 irit.ctlpt( irit.E3, 0.2, 2.55, 0.112 ), \
                                                 irit.ctlpt( irit.E3, 0.4, 2.4, 0.224 ), \
                                                 irit.ctlpt( irit.E3, 1.3, 2.4, 0.728 ), \
                                                 irit.ctlpt( irit.E3, 1.3, 2.25, 0.728 ) ), irit.list( \
                                                 irit.ctlpt( irit.E3, 0.002, 3, 0 ), \
                                                 irit.ctlpt( irit.E3, 0.45, 3, 0.8 ), \
                                                 irit.ctlpt( irit.E3, 0, 2.7, 0 ), \
                                                 irit.ctlpt( irit.E3, 0.112, 2.55, 0.2 ), \
                                                 irit.ctlpt( irit.E3, 0.224, 2.4, 0.4 ), \
                                                 irit.ctlpt( irit.E3, 0.728, 2.4, 1.3 ), \
                                                 irit.ctlpt( irit.E3, 0.728, 2.25, 1.3 ) ), irit.list( \
                                                 irit.ctlpt( irit.E3, 0, 3, 0 ), \
                                                 irit.ctlpt( irit.E3, 0, 3, 0.8 ), \
                                                 irit.ctlpt( irit.E3, 0, 2.7, 0 ), \
                                                 irit.ctlpt( irit.E3, 0, 2.55, 0.2 ), \
                                                 irit.ctlpt( irit.E3, 0, 2.4, 0.4 ), \
                                                 irit.ctlpt( irit.E3, 0, 2.4, 1.3 ), \
                                                 irit.ctlpt( irit.E3, 0, 2.25, 1.3 ) ), irit.list( \
                                                 irit.ctlpt( irit.E3, (-0.002 ), 3, 0 ), \
                                                 irit.ctlpt( irit.E3, (-0.45 ), 3, 0.8 ), \
                                                 irit.ctlpt( irit.E3, 0, 2.7, 0 ), \
                                                 irit.ctlpt( irit.E3, (-0.112 ), 2.55, 0.2 ), \
                                                 irit.ctlpt( irit.E3, (-0.224 ), 2.4, 0.4 ), \
                                                 irit.ctlpt( irit.E3, (-0.728 ), 2.4, 1.3 ), \
                                                 irit.ctlpt( irit.E3, (-0.728 ), 2.25, 1.3 ) ), irit.list( \
                                                 irit.ctlpt( irit.E3, 0, 3, 0.002 ), \
                                                 irit.ctlpt( irit.E3, (-0.8 ), 3, 0.45 ), \
                                                 irit.ctlpt( irit.E3, 0, 2.7, 0 ), \
                                                 irit.ctlpt( irit.E3, (-0.2 ), 2.55, 0.112 ), \
                                                 irit.ctlpt( irit.E3, (-0.4 ), 2.4, 0.224 ), \
                                                 irit.ctlpt( irit.E3, (-1.3 ), 2.4, 0.728 ), \
                                                 irit.ctlpt( irit.E3, (-1.3 ), 2.25, 0.728 ) ), irit.list( \
                                                 irit.ctlpt( irit.E3, 0, 3, 0 ), \
                                                 irit.ctlpt( irit.E3, (-0.8 ), 3, 0 ), \
                                                 irit.ctlpt( irit.E3, 0, 2.7, 0 ), \
                                                 irit.ctlpt( irit.E3, (-0.2 ), 2.55, 0 ), \
                                                 irit.ctlpt( irit.E3, (-0.4 ), 2.4, 0 ), \
                                                 irit.ctlpt( irit.E3, (-1.3 ), 2.4, 0 ), \
                                                 irit.ctlpt( irit.E3, (-1.3 ), 2.25, 0 ) ), irit.list( \
                                                 irit.ctlpt( irit.E3, 0, 3, (-0.002 ) ), \
                                                 irit.ctlpt( irit.E3, (-0.8 ), 3, (-0.45 ) ), \
                                                 irit.ctlpt( irit.E3, 0, 2.7, 0 ), \
                                                 irit.ctlpt( irit.E3, (-0.2 ), 2.55, (-0.112 ) ), \
                                                 irit.ctlpt( irit.E3, (-0.4 ), 2.4, (-0.224 ) ), \
                                                 irit.ctlpt( irit.E3, (-1.3 ), 2.4, (-0.728 ) ), \
                                                 irit.ctlpt( irit.E3, (-1.3 ), 2.25, (-0.728 ) ) ), irit.list( \
                                                 irit.ctlpt( irit.E3, (-0.002 ), 3, 0 ), \
                                                 irit.ctlpt( irit.E3, (-0.45 ), 3, (-0.8 ) ), \
                                                 irit.ctlpt( irit.E3, 0, 2.7, 0 ), \
                                                 irit.ctlpt( irit.E3, (-0.112 ), 2.55, (-0.2 ) ), \
                                                 irit.ctlpt( irit.E3, (-0.224 ), 2.4, (-0.4 ) ), \
                                                 irit.ctlpt( irit.E3, (-0.728 ), 2.4, (-1.3 ) ), \
                                                 irit.ctlpt( irit.E3, (-0.728 ), 2.25, (-1.3 ) ) ), irit.list( \
                                                 irit.ctlpt( irit.E3, 0, 3, 0 ), \
                                                 irit.ctlpt( irit.E3, 0, 3, (-0.8 ) ), \
                                                 irit.ctlpt( irit.E3, 0, 2.7, 0 ), \
                                                 irit.ctlpt( irit.E3, 0, 2.55, (-0.2 ) ), \
                                                 irit.ctlpt( irit.E3, 0, 2.4, (-0.4 ) ), \
                                                 irit.ctlpt( irit.E3, 0, 2.4, (-1.3 ) ), \
                                                 irit.ctlpt( irit.E3, 0, 2.25, (-1.3 ) ) ), irit.list( \
                                                 irit.ctlpt( irit.E3, 0.002, 3, 0 ), \
                                                 irit.ctlpt( irit.E3, 0.45, 3, (-0.8 ) ), \
                                                 irit.ctlpt( irit.E3, 0, 2.7, 0 ), \
                                                 irit.ctlpt( irit.E3, 0.112, 2.55, (-0.2 ) ), \
                                                 irit.ctlpt( irit.E3, 0.224, 2.4, (-0.4 ) ), \
                                                 irit.ctlpt( irit.E3, 0.728, 2.4, (-1.3 ) ), \
                                                 irit.ctlpt( irit.E3, 0.728, 2.25, (-1.3 ) ) ), irit.list( \
                                                 irit.ctlpt( irit.E3, 0, 3, (-0.002 ) ), \
                                                 irit.ctlpt( irit.E3, 0.8, 3, (-0.45 ) ), \
                                                 irit.ctlpt( irit.E3, 0, 2.7, 0 ), \
                                                 irit.ctlpt( irit.E3, 0.2, 2.55, (-0.112 ) ), \
                                                 irit.ctlpt( irit.E3, 0.4, 2.4, (-0.224 ) ), \
                                                 irit.ctlpt( irit.E3, 1.3, 2.4, (-0.728 ) ), \
                                                 irit.ctlpt( irit.E3, 1.3, 2.25, (-0.728 ) ) ), irit.list( \
                                                 irit.ctlpt( irit.E3, 0, 3, 0 ), \
                                                 irit.ctlpt( irit.E3, 0.8, 3, 0 ), \
                                                 irit.ctlpt( irit.E3, 0, 2.7, 0 ), \
                                                 irit.ctlpt( irit.E3, 0.2, 2.55, 0 ), \
                                                 irit.ctlpt( irit.E3, 0.4, 2.4, 0 ), \
                                                 irit.ctlpt( irit.E3, 1.3, 2.4, 0 ), \
                                                 irit.ctlpt( irit.E3, 1.3, 2.25, 0 ) ) ), irit.list( irit.list( 0, 0, 0, 0, 1, 1,\
1, 2, 2, 2, 2 ), irit.list( 0, 0, 0, 0, 1, 1,\
1, 2, 2, 2, 3, 3,\
3, 4, 4, 4, 4 ) ) )

echosrc = irit.iritstate( "echosource", echosrc )
irit.free( echosrc )

#  Faster product using Bezier decomposition.
iprod = irit.iritstate( "bspprodmethod", irit.GenRealObject(0) )

# 
#  Render the following using irender to get the curvature bound as color map
#  on top of the teapot.
# 
irit.color( body, irit.RED )
irit.attrib( body, "stexture", irit.smean( body, 0 ) )
irit.attrib( body, "stexture_func", irit.GenStrObject("sqrt" ) )
irit.attrib( body, "stexture_bound", irit.GenStrObject("-5.0 20.0" ) )

irit.color( cap, irit.GREEN )
irit.attrib( cap, "stexture", irit.smean( cap, 0 ) )
irit.attrib( cap, "stexture_func", irit.GenStrObject("sqrt" ) )
irit.attrib( cap, "stexture_bound", irit.GenStrObject("-5.0 20.0" ) )

irit.color( spout, irit.BLUE )
irit.attrib( spout, "stexture", irit.smean( spout, 0 ) )
irit.attrib( spout, "stexture_func", irit.GenStrObject("sqrt" ) )
irit.attrib( spout, "stexture_bound", irit.GenStrObject("-5.0 20.0" ) )

irit.color( handle, irit.MAGENTA )
irit.attrib( handle, "stexture", irit.smean( handle, 0 ) )
irit.attrib( handle, "stexture_func", irit.GenStrObject("sqrt" ) )
irit.attrib( handle, "stexture_bound", irit.GenStrObject("-5.0 20.0" ) )

teapot = irit.list( body, spout, handle, cap )

irit.interact( irit.list( irit.GetViewMatrix(), teapot ) )
irit.save( "meanstea", teapot )

# 
#  Render the following using irender to get the curvature bound as color map
#  on top of the teapot.
# 
irit.color( body, irit.RED )
irit.attrib( body, "stexture", irit.sgauss( body, 0 ) )
irit.attrib( body, "stexture_bound", irit.GenStrObject("-5.0 10.0" ) )

irit.color( cap, irit.GREEN )
irit.attrib( cap, "stexture", irit.sgauss( cap, 0 ) )
irit.attrib( cap, "stexture_bound", irit.GenStrObject("-5.0 10.0" ) )

irit.color( spout, irit.BLUE )
irit.attrib( spout, "stexture", irit.sgauss( spout, 0 ) )
irit.attrib( spout, "stexture_bound", irit.GenStrObject("-5.0 10.0" ))

irit.color( handle, irit.MAGENTA )
irit.attrib( handle, "stexture", irit.sgauss( handle, 0 ) )
irit.attrib( handle, "stexture_bound", irit.GenStrObject("-5.0 10.0" ))

teapot = irit.list( body, spout, handle, cap )

irit.interact( irit.list( irit.GetViewMatrix(), teapot ) )
irit.save( "gausstea", teapot )

# 
#  Render the following using irender to get the curvature bound as color map
#  on top of the teapot.
# 
irit.color( body, irit.RED )
irit.attrib( body, "stexture", irit.scrvtr( body, irit.P1, 0 ) )
irit.attrib( body, "stexture_func", irit.GenStrObject("sqrt" ) )
irit.attrib( body, "stexture_bound", irit.GenStrObject("0.0 30.0" ) )

irit.color( cap, irit.GREEN )
irit.attrib( cap, "stexture", irit.scrvtr( cap, irit.P1, 0 ) )
irit.attrib( cap, "stexture_func", irit.GenStrObject("sqrt" ) )
irit.attrib( cap, "stexture_bound", irit.GenStrObject("0.0 30.0" ) )

irit.color( spout, irit.BLUE )
irit.attrib( spout, "stexture", irit.scrvtr( spout, irit.P1, 0 ) )
irit.attrib( spout, "stexture_func", irit.GenStrObject("sqrt" ) )
irit.attrib( spout, "stexture_bound", irit.GenStrObject("0.0 30.0" ) )

irit.color( handle, irit.MAGENTA )
irit.attrib( handle, "stexture", irit.scrvtr( handle, irit.P1, 0 ) )
irit.attrib( handle, "stexture_func", irit.GenStrObject("sqrt" ) )
irit.attrib( handle, "stexture_bound", irit.GenStrObject("0.0 30.0" ) )

teapot = irit.list( body, spout, handle, cap )

irit.interact( irit.list( irit.GetViewMatrix(), teapot ) )
irit.save( "crvtrtea", teapot )

iprod = irit.iritstate( "bspprodmethod", iprod )
irit.free( iprod )

