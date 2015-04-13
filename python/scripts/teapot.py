#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  The infamous Teapot data as four B-spline surfaces.
# 

echosrc = irit.iritstate( "echosource", irit.GenRealObject(0) )
save_mat = irit.GetViewMatrix()

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

cap = irit.sreparam( irit.sregion( cap, irit.COL, 0.0005, 2 ), irit.COL, 0,\
2 )


echosrc = irit.iritstate( "echosource", echosrc )
irit.free( echosrc )

irit.color( body, irit.RED )
irit.color( cap, irit.GREEN )
irit.color( spout, irit.BLUE )
irit.color( handle, irit.MAGENTA )

irit.SetViewMatrix(  irit.scale( ( 0.3, 0.3, 0.3 ) ))
teapot = irit.list( body, spout, handle, cap )

# interact( list( view_mat, Teapot ) );
irit.save( "teapot", teapot )

tea1 = teapot * irit.tx( 7 ) 
tea2 = teapot * irit.tx( 7 ) * irit.tz( 5 ) 
tea3 = teapot * irit.tz( 5 ) 

irit.SetViewMatrix(  irit.scale( ( 0.15, 0.15, 0.15 ) ) * irit.rx( 50 ) * irit.ry( 40 ) * irit.tx( (-0.7 ) ) * irit.ty( 0.2 ))
irit.interact( irit.list( irit.GetViewMatrix(), teapot, tea1, tea2, tea3 ) )
irit.free( tea1 )
irit.free( tea2 )
irit.free( tea3 )

# 
#  Extract Isocurves from the teapot surfaces.
# 

# 
#  Body
# 
tmp = irit.csurface( body, irit.COL, 0 )
irit.color( tmp, irit.YELLOW )
bodyiso = irit.list( tmp )
t = 0.3
while ( t <= 3.01 ):
    tmp = irit.csurface( body, irit.COL, t )
    irit.color( tmp, irit.YELLOW )
    irit.snoc( tmp, bodyiso )
    t = t + 0.3
t = 0.3
while ( t <= 3.91 ):
    tmp = irit.csurface( body, irit.ROW, t )
    irit.color( tmp, irit.YELLOW )
    irit.snoc( tmp, bodyiso )
    t = t + 0.3

# 
#  Spout
# 
tmp = irit.csurface( spout, irit.COL, 0 )
irit.color( tmp, irit.YELLOW )
spoutiso = irit.list( tmp )
t = 0.2
while ( t <= 2.01 ):
    tmp = irit.csurface( spout, irit.COL, t )
    irit.color( tmp, irit.YELLOW )
    irit.snoc( tmp, spoutiso )
    t = t + 0.2
t = 0.25
while ( t <= 2.01 ):
    tmp = irit.csurface( spout, irit.ROW, t )
    irit.color( tmp, irit.YELLOW )
    irit.snoc( tmp, spoutiso )
    t = t + 0.25

# 
#  Handle
# 
tmp = irit.csurface( handle, irit.COL, 0 )
irit.color( tmp, irit.YELLOW )
handleiso = irit.list( tmp )
t = 0.2
while ( t <= 2.01 ):
    tmp = irit.csurface( handle, irit.COL, t )
    irit.color( tmp, irit.YELLOW )
    irit.snoc( tmp, handleiso )
    t = t + 0.2
t = 0.4
while ( t <= 2.01 ):
    tmp = irit.csurface( handle, irit.ROW, t )
    irit.color( tmp, irit.YELLOW )
    irit.snoc( tmp, handleiso )
    t = t + 0.4

# 
#  Cap
# 
tmp = irit.csurface( cap, irit.COL, 0 )
irit.color( tmp, irit.YELLOW )
capiso = irit.list( tmp )
t = 0.25
while ( t <= 2.01 ):
    tmp = irit.csurface( cap, irit.COL, t )
    irit.color( tmp, irit.YELLOW )
    irit.snoc( tmp, capiso )
    t = t + 0.25
t = 0.4
while ( t <= 4.01 ):
    tmp = irit.csurface( cap, irit.ROW, t )
    irit.color( tmp, irit.YELLOW )
    irit.snoc( tmp, capiso )
    t = t + 0.4


teapotiso = irit.list( bodyiso, spoutiso, handleiso, capiso )
irit.SetViewMatrix(  irit.scale( ( 0.3, 0.3, 0.3 ) ) * irit.rx( 50 ) * irit.ry( 40 ))
irit.interact( irit.list( irit.GetViewMatrix(), teapotiso ) )
irit.save( "teapot-iso", teapotiso )

irit.SetViewMatrix(  save_mat)

irit.free( body )
irit.free( spout )
irit.free( handle )
irit.free( cap )
irit.free( teapot )
irit.free( tmp )
irit.free( bodyiso )
irit.free( spoutiso )
irit.free( handleiso )
irit.free( capiso )
irit.free( teapotiso )

