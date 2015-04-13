#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Deformed letters "CNC"
# 

cnc_c1 = irit.list( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, (-0.38441 ), 0.46487 ), \
                                                 irit.ctlpt( irit.E2, (-0.35417 ), 0.31561 ), \
                                                 irit.ctlpt( irit.E2, (-0.32438 ), 0.16635 ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 3, irit.list( \
                                                 irit.ctlpt( irit.E2, (-0.32438 ), 0.16635 ), \
                                                 irit.ctlpt( irit.E2, (-0.33111 ), 0.16614 ), \
                                                 irit.ctlpt( irit.E2, (-0.33784 ), 0.16593 ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 5, irit.list( \
                                                 irit.ctlpt( irit.E2, (-0.33784 ), 0.16593 ), \
                                                 irit.ctlpt( irit.E2, (-0.35753 ), 0.22412 ), \
                                                 irit.ctlpt( irit.E2, (-0.38038 ), 0.27308 ), \
                                                 irit.ctlpt( irit.E2, (-0.40698 ), 0.3128 ), \
                                                 irit.ctlpt( irit.E2, (-0.4371 ), 0.34328 ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 5, irit.list( \
                                                 irit.ctlpt( irit.E2, (-0.4371 ), 0.34328 ), \
                                                 irit.ctlpt( irit.E2, (-0.46722 ), 0.37377 ), \
                                                 irit.ctlpt( irit.E2, (-0.4978 ), 0.39382 ), \
                                                 irit.ctlpt( irit.E2, (-0.5283 ), 0.40344 ), \
                                                 irit.ctlpt( irit.E2, (-0.55782 ), 0.40262 ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 5, irit.list( \
                                                 irit.ctlpt( irit.E2, (-0.55782 ), 0.40262 ), \
                                                 irit.ctlpt( irit.E2, (-0.58249 ), 0.40195 ), \
                                                 irit.ctlpt( irit.E2, (-0.60353 ), 0.39358 ), \
                                                 irit.ctlpt( irit.E2, (-0.6204 ), 0.37751 ), \
                                                 irit.ctlpt( irit.E2, (-0.63274 ), 0.35374 ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 5, irit.list( \
                                                 irit.ctlpt( irit.E2, (-0.63274 ), 0.35374 ), \
                                                 irit.ctlpt( irit.E2, (-0.64509 ), 0.32997 ), \
                                                 irit.ctlpt( irit.E2, (-0.65163 ), 0.30154 ), \
                                                 irit.ctlpt( irit.E2, (-0.65235 ), 0.26844 ), \
                                                 irit.ctlpt( irit.E2, (-0.64753 ), 0.23068 ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 5, irit.list( \
                                                 irit.ctlpt( irit.E2, (-0.64753 ), 0.23068 ), \
                                                 irit.ctlpt( irit.E2, (-0.64141 ), 0.18235 ), \
                                                 irit.ctlpt( irit.E2, (-0.62953 ), 0.13053 ), \
                                                 irit.ctlpt( irit.E2, (-0.61235 ), 0.07522 ), \
                                                 irit.ctlpt( irit.E2, (-0.59057 ), 0.01643 ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 5, irit.list( \
                                                 irit.ctlpt( irit.E2, (-0.59057 ), 0.01643 ), \
                                                 irit.ctlpt( irit.E2, (-0.56914 ), (-0.04142 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.54661 ), (-0.09573 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.52347 ), (-0.14652 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.50002 ), (-0.19377 ) ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 5, irit.list( \
                                                 irit.ctlpt( irit.E2, (-0.50002 ), (-0.19377 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.47657 ), (-0.24102 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.45407 ), (-0.28044 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.43276 ), (-0.31203 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.41244 ), (-0.33578 ) ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 5, irit.list( \
                                                 irit.ctlpt( irit.E2, (-0.41244 ), (-0.33578 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.39211 ), (-0.35954 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.37263 ), (-0.37508 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.35381 ), (-0.38241 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.3349 ), (-0.38153 ) ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 5, irit.list( \
                                                 irit.ctlpt( irit.E2, (-0.3349 ), (-0.38153 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.31939 ), (-0.38081 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.30641 ), (-0.37419 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.29554 ), (-0.36169 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.28651 ), (-0.3433 ) ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 5, irit.list( \
                                                 irit.ctlpt( irit.E2, (-0.28651 ), (-0.3433 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.27748 ), (-0.32491 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.26955 ), (-0.29792 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.26218 ), (-0.26234 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.25473 ), (-0.21817 ) ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 3, irit.list( \
                                                 irit.ctlpt( irit.E2, (-0.25473 ), (-0.21817 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.2473 ), (-0.25533 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.2399 ), (-0.29249 ) ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 5, irit.list( \
                                                 irit.ctlpt( irit.E2, (-0.2399 ), (-0.29249 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.24718 ), (-0.33092 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.25519 ), (-0.36244 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.26428 ), (-0.38705 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.27489 ), (-0.40474 ) ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 5, irit.list( \
                                                 irit.ctlpt( irit.E2, (-0.27489 ), (-0.40474 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.2855 ), (-0.42244 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.29859 ), (-0.43455 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.31441 ), (-0.44107 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.33346 ), (-0.44201 ) ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 5, irit.list( \
                                                 irit.ctlpt( irit.E2, (-0.33346 ), (-0.44201 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.35852 ), (-0.44324 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.38437 ), (-0.43517 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.41215 ), (-0.41779 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.44251 ), (-0.3911 ) ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 5, irit.list( \
                                                 irit.ctlpt( irit.E2, (-0.44251 ), (-0.3911 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.47287 ), (-0.3644 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.50425 ), (-0.32951 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.53705 ), (-0.28641 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.57096 ), (-0.2351 ) ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 5, irit.list( \
                                                 irit.ctlpt( irit.E2, (-0.57096 ), (-0.2351 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.60488 ), (-0.1838 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.63685 ), (-0.1301 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.66611 ), (-0.074 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.69168 ), (-0.01551 ) ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 5, irit.list( \
                                                 irit.ctlpt( irit.E2, (-0.69168 ), (-0.01551 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.71863 ), 0.04616 ), \
                                                 irit.ctlpt( irit.E2, (-0.73949 ), 0.1059 ), \
                                                 irit.ctlpt( irit.E2, (-0.75307 ), 0.1637 ), \
                                                 irit.ctlpt( irit.E2, (-0.75843 ), 0.21956 ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 5, irit.list( \
                                                 irit.ctlpt( irit.E2, (-0.75843 ), 0.21956 ), \
                                                 irit.ctlpt( irit.E2, (-0.76379 ), 0.27542 ), \
                                                 irit.ctlpt( irit.E2, (-0.76013 ), 0.32314 ), \
                                                 irit.ctlpt( irit.E2, (-0.74715 ), 0.36271 ), \
                                                 irit.ctlpt( irit.E2, (-0.72519 ), 0.39415 ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 5, irit.list( \
                                                 irit.ctlpt( irit.E2, (-0.72519 ), 0.39415 ), \
                                                 irit.ctlpt( irit.E2, (-0.70323 ), 0.42558 ), \
                                                 irit.ctlpt( irit.E2, (-0.67478 ), 0.44688 ), \
                                                 irit.ctlpt( irit.E2, (-0.64067 ), 0.45804 ), \
                                                 irit.ctlpt( irit.E2, (-0.60198 ), 0.45907 ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 5, irit.list( \
                                                 irit.ctlpt( irit.E2, (-0.60198 ), 0.45907 ), \
                                                 irit.ctlpt( irit.E2, (-0.57346 ), 0.45984 ), \
                                                 irit.ctlpt( irit.E2, (-0.54186 ), 0.45406 ), \
                                                 irit.ctlpt( irit.E2, (-0.50769 ), 0.44175 ), \
                                                 irit.ctlpt( irit.E2, (-0.4716 ), 0.42291 ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 5, irit.list( \
                                                 irit.ctlpt( irit.E2, (-0.4716 ), 0.42291 ), \
                                                 irit.ctlpt( irit.E2, (-0.45071 ), 0.41197 ), \
                                                 irit.ctlpt( irit.E2, (-0.43539 ), 0.40472 ), \
                                                 irit.ctlpt( irit.E2, (-0.42533 ), 0.40116 ), \
                                                 irit.ctlpt( irit.E2, (-0.42046 ), 0.40129 ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 5, irit.list( \
                                                 irit.ctlpt( irit.E2, (-0.42046 ), 0.40129 ), \
                                                 irit.ctlpt( irit.E2, (-0.41424 ), 0.40146 ), \
                                                 irit.ctlpt( irit.E2, (-0.40913 ), 0.40411 ), \
                                                 irit.ctlpt( irit.E2, (-0.40508 ), 0.40922 ), \
                                                 irit.ctlpt( irit.E2, (-0.40208 ), 0.41681 ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 5, irit.list( \
                                                 irit.ctlpt( irit.E2, (-0.40208 ), 0.41681 ), \
                                                 irit.ctlpt( irit.E2, (-0.39908 ), 0.4244 ), \
                                                 irit.ctlpt( irit.E2, (-0.39775 ), 0.43487 ), \
                                                 irit.ctlpt( irit.E2, (-0.39805 ), 0.44823 ), \
                                                 irit.ctlpt( irit.E2, (-0.40005 ), 0.46446 ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 3, irit.list( \
                                                 irit.ctlpt( irit.E2, (-0.40005 ), 0.46446 ), \
                                                 irit.ctlpt( irit.E2, (-0.39223 ), 0.46467 ), \
                                                 irit.ctlpt( irit.E2, (-0.38441 ), 0.46487 ) ), irit.list( irit.KV_OPEN ) ) )

cnc_n2 = irit.list( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, (-0.13101 ), 0.43544 ), \
                                                 irit.ctlpt( irit.E2, 0.02482, 0.17205 ), \
                                                 irit.ctlpt( irit.E2, 0.11351, (-0.09106 ) ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 3, irit.list( \
                                                 irit.ctlpt( irit.E2, 0.11351, (-0.09106 ) ), \
                                                 irit.ctlpt( irit.E2, 0.12162, 0.0939 ), \
                                                 irit.ctlpt( irit.E2, 0.12903, 0.27887 ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 5, irit.list( \
                                                 irit.ctlpt( irit.E2, 0.12903, 0.27887 ), \
                                                 irit.ctlpt( irit.E2, 0.13058, 0.31757 ), \
                                                 irit.ctlpt( irit.E2, 0.12962, 0.34786 ), \
                                                 irit.ctlpt( irit.E2, 0.12592, 0.36973 ), \
                                                 irit.ctlpt( irit.E2, 0.11957, 0.38318 ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 5, irit.list( \
                                                 irit.ctlpt( irit.E2, 0.11957, 0.38318 ), \
                                                 irit.ctlpt( irit.E2, 0.11084, 0.40133 ), \
                                                 irit.ctlpt( irit.E2, 0.09747, 0.41302 ), \
                                                 irit.ctlpt( irit.E2, 0.07943, 0.41824 ), \
                                                 irit.ctlpt( irit.E2, 0.05714, 0.417 ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 3, irit.list( \
                                                 irit.ctlpt( irit.E2, 0.05714, 0.417 ), \
                                                 irit.ctlpt( irit.E2, 0.05717, 0.42874 ), \
                                                 irit.ctlpt( irit.E2, 0.05719, 0.44048 ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 3, irit.list( \
                                                 irit.ctlpt( irit.E2, 0.05719, 0.44048 ), \
                                                 irit.ctlpt( irit.E2, 0.14654, 0.44287 ), \
                                                 irit.ctlpt( irit.E2, 0.2363, 0.44525 ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 3, irit.list( \
                                                 irit.ctlpt( irit.E2, 0.2363, 0.44525 ), \
                                                 irit.ctlpt( irit.E2, 0.2353, 0.43352 ), \
                                                 irit.ctlpt( irit.E2, 0.23429, 0.42178 ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 5, irit.list( \
                                                 irit.ctlpt( irit.E2, 0.23429, 0.42178 ), \
                                                 irit.ctlpt( irit.E2, 0.21695, 0.41784 ), \
                                                 irit.ctlpt( irit.E2, 0.20337, 0.4133 ), \
                                                 irit.ctlpt( irit.E2, 0.19354, 0.40818 ), \
                                                 irit.ctlpt( irit.E2, 0.18739, 0.40247 ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 5, irit.list( \
                                                 irit.ctlpt( irit.E2, 0.18739, 0.40247 ), \
                                                 irit.ctlpt( irit.E2, 0.18124, 0.39676 ), \
                                                 irit.ctlpt( irit.E2, 0.17584, 0.38875 ), \
                                                 irit.ctlpt( irit.E2, 0.1712, 0.37843 ), \
                                                 irit.ctlpt( irit.E2, 0.16732, 0.36582 ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 5, irit.list( \
                                                 irit.ctlpt( irit.E2, 0.16732, 0.36582 ), \
                                                 irit.ctlpt( irit.E2, 0.16343, 0.3532 ), \
                                                 irit.ctlpt( irit.E2, 0.16032, 0.33463 ), \
                                                 irit.ctlpt( irit.E2, 0.158, 0.31012 ), \
                                                 irit.ctlpt( irit.E2, 0.15636, 0.27967 ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 3, irit.list( \
                                                 irit.ctlpt( irit.E2, 0.15636, 0.27967 ), \
                                                 irit.ctlpt( irit.E2, 0.13707, (-0.0785 ) ), \
                                                 irit.ctlpt( irit.E2, 0.11514, (-0.43665 ) ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 3, irit.list( \
                                                 irit.ctlpt( irit.E2, 0.11514, (-0.43665 ) ), \
                                                 irit.ctlpt( irit.E2, 0.11113, (-0.43685 ) ), \
                                                 irit.ctlpt( irit.E2, 0.10712, (-0.43704 ) ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 3, irit.list( \
                                                 irit.ctlpt( irit.E2, 0.10712, (-0.43704 ) ), \
                                                 irit.ctlpt( irit.E2, 0.01873, (-0.08408 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.19311 ), 0.2694 ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 3, irit.list( \
                                                 irit.ctlpt( irit.E2, (-0.19311 ), 0.2694 ), \
                                                 irit.ctlpt( irit.E2, (-0.15932 ), (-0.00426 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.12707 ), (-0.27793 ) ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 5, irit.list( \
                                                 irit.ctlpt( irit.E2, (-0.12707 ), (-0.27793 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.12269 ), (-0.31507 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.11693 ), (-0.34407 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.11014 ), (-0.36493 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.1022 ), (-0.37764 ) ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 5, irit.list( \
                                                 irit.ctlpt( irit.E2, (-0.1022 ), (-0.37764 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.09425 ), (-0.39035 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.08631 ), (-0.3987 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.07827 ), (-0.40267 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.06997 ), (-0.40229 ) ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 3, irit.list( \
                                                 irit.ctlpt( irit.E2, (-0.06997 ), (-0.40229 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.06612 ), (-0.4021 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.06227 ), (-0.40192 ) ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 3, irit.list( \
                                                 irit.ctlpt( irit.E2, (-0.06227 ), (-0.40192 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.06135 ), (-0.41366 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.06044 ), (-0.42541 ) ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 3, irit.list( \
                                                 irit.ctlpt( irit.E2, (-0.06044 ), (-0.42541 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.1173 ), (-0.42814 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.17367 ), (-0.43087 ) ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 3, irit.list( \
                                                 irit.ctlpt( irit.E2, (-0.17367 ), (-0.43087 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.17564 ), (-0.41912 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.1776 ), (-0.40737 ) ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 5, irit.list( \
                                                 irit.ctlpt( irit.E2, (-0.1776 ), (-0.40737 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.16428 ), (-0.40641 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.15442 ), (-0.40093 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.14775 ), (-0.39092 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.14435 ), (-0.37639 ) ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 5, irit.list( \
                                                 irit.ctlpt( irit.E2, (-0.14435 ), (-0.37639 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.14094 ), (-0.36185 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.1401 ), (-0.34073 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.14179 ), (-0.31303 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.14629 ), (-0.27874 ) ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 3, irit.list( \
                                                 irit.ctlpt( irit.E2, (-0.14629 ), (-0.27874 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.18624 ), 0.0251 ), \
                                                 irit.ctlpt( irit.E2, (-0.22808 ), 0.32894 ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 3, irit.list( \
                                                 irit.ctlpt( irit.E2, (-0.22808 ), 0.32894 ), \
                                                 irit.ctlpt( irit.E2, (-0.23511 ), 0.34021 ), \
                                                 irit.ctlpt( irit.E2, (-0.24227 ), 0.35148 ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 5, irit.list( \
                                                 irit.ctlpt( irit.E2, (-0.24227 ), 0.35148 ), \
                                                 irit.ctlpt( irit.E2, (-0.2529 ), 0.3684 ), \
                                                 irit.ctlpt( irit.E2, (-0.26253 ), 0.38141 ), \
                                                 irit.ctlpt( irit.E2, (-0.27102 ), 0.39053 ), \
                                                 irit.ctlpt( irit.E2, (-0.27829 ), 0.39576 ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 5, irit.list( \
                                                 irit.ctlpt( irit.E2, (-0.27829 ), 0.39576 ), \
                                                 irit.ctlpt( irit.E2, (-0.28555 ), 0.40098 ), \
                                                 irit.ctlpt( irit.E2, (-0.29449 ), 0.40456 ), \
                                                 irit.ctlpt( irit.E2, (-0.3051 ), 0.4065 ), \
                                                 irit.ctlpt( irit.E2, (-0.31732 ), 0.4068 ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 3, irit.list( \
                                                 irit.ctlpt( irit.E2, (-0.31732 ), 0.4068 ), \
                                                 irit.ctlpt( irit.E2, (-0.31938 ), 0.41855 ), \
                                                 irit.ctlpt( irit.E2, (-0.32145 ), 0.4303 ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 3, irit.list( \
                                                 irit.ctlpt( irit.E2, (-0.32145 ), 0.4303 ), \
                                                 irit.ctlpt( irit.E2, (-0.22647 ), 0.43287 ), \
                                                 irit.ctlpt( irit.E2, (-0.13101 ), 0.43544 ) ), irit.list( irit.KV_OPEN ) ) )

cnc_c3 = irit.list( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, 0.80862, 0.46361 ), \
                                                 irit.ctlpt( irit.E2, 0.75653, 0.3147 ), \
                                                 irit.ctlpt( irit.E2, 0.70398, 0.16578 ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 3, irit.list( \
                                                 irit.ctlpt( irit.E2, 0.70398, 0.16578 ), \
                                                 irit.ctlpt( irit.E2, 0.69713, 0.16557 ), \
                                                 irit.ctlpt( irit.E2, 0.69028, 0.16536 ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 5, irit.list( \
                                                 irit.ctlpt( irit.E2, 0.69028, 0.16536 ), \
                                                 irit.ctlpt( irit.E2, 0.70269, 0.22342 ), \
                                                 irit.ctlpt( irit.E2, 0.70684, 0.27226 ), \
                                                 irit.ctlpt( irit.E2, 0.70215, 0.31189 ), \
                                                 irit.ctlpt( irit.E2, 0.68883, 0.34231 ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 5, irit.list( \
                                                 irit.ctlpt( irit.E2, 0.68883, 0.34231 ), \
                                                 irit.ctlpt( irit.E2, 0.6755, 0.37272 ), \
                                                 irit.ctlpt( irit.E2, 0.65597, 0.39273 ), \
                                                 irit.ctlpt( irit.E2, 0.63076, 0.40232 ), \
                                                 irit.ctlpt( irit.E2, 0.60078, 0.40151 ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 5, irit.list( \
                                                 irit.ctlpt( irit.E2, 0.60078, 0.40151 ), \
                                                 irit.ctlpt( irit.E2, 0.57572, 0.40083 ), \
                                                 irit.ctlpt( irit.E2, 0.55005, 0.39248 ), \
                                                 irit.ctlpt( irit.E2, 0.52431, 0.37645 ), \
                                                 irit.ctlpt( irit.E2, 0.49885, 0.35274 ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 5, irit.list( \
                                                 irit.ctlpt( irit.E2, 0.49885, 0.35274 ), \
                                                 irit.ctlpt( irit.E2, 0.47339, 0.32902 ), \
                                                 irit.ctlpt( irit.E2, 0.45117, 0.30066 ), \
                                                 irit.ctlpt( irit.E2, 0.4322, 0.26764 ), \
                                                 irit.ctlpt( irit.E2, 0.4162, 0.22996 ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 5, irit.list( \
                                                 irit.ctlpt( irit.E2, 0.4162, 0.22996 ), \
                                                 irit.ctlpt( irit.E2, 0.39568, 0.18174 ), \
                                                 irit.ctlpt( irit.E2, 0.37899, 0.13004 ), \
                                                 irit.ctlpt( irit.E2, 0.36568, 0.07486 ), \
                                                 irit.ctlpt( irit.E2, 0.35505, 0.0162 ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 5, irit.list( \
                                                 irit.ctlpt( irit.E2, 0.35505, 0.0162 ), \
                                                 irit.ctlpt( irit.E2, 0.34459, (-0.04151 ) ), \
                                                 irit.ctlpt( irit.E2, 0.33717, (-0.0957 ) ), \
                                                 irit.ctlpt( irit.E2, 0.33232, (-0.14636 ) ), \
                                                 irit.ctlpt( irit.E2, 0.32972, (-0.19351 ) ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 5, irit.list( \
                                                 irit.ctlpt( irit.E2, 0.32972, (-0.19351 ) ), \
                                                 irit.ctlpt( irit.E2, 0.32712, (-0.24065 ) ), \
                                                 irit.ctlpt( irit.E2, 0.32789, (-0.27998 ) ), \
                                                 irit.ctlpt( irit.E2, 0.33177, (-0.31149 ) ), \
                                                 irit.ctlpt( irit.E2, 0.339, (-0.33519 ) ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 5, irit.list( \
                                                 irit.ctlpt( irit.E2, 0.339, (-0.33519 ) ), \
                                                 irit.ctlpt( irit.E2, 0.34622, (-0.3589 ) ), \
                                                 irit.ctlpt( irit.E2, 0.35712, (-0.3744 ) ), \
                                                 irit.ctlpt( irit.E2, 0.3719, (-0.38172 ) ), \
                                                 irit.ctlpt( irit.E2, 0.39128, (-0.38084 ) ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 5, irit.list( \
                                                 irit.ctlpt( irit.E2, 0.39128, (-0.38084 ) ), \
                                                 irit.ctlpt( irit.E2, 0.40718, (-0.38011 ) ), \
                                                 irit.ctlpt( irit.E2, 0.4238, (-0.37351 ) ), \
                                                 irit.ctlpt( irit.E2, 0.44156, (-0.36104 ) ), \
                                                 irit.ctlpt( irit.E2, 0.46073, (-0.34269 ) ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 5, irit.list( \
                                                 irit.ctlpt( irit.E2, 0.46073, (-0.34269 ) ), \
                                                 irit.ctlpt( irit.E2, 0.47989, (-0.32434 ) ), \
                                                 irit.ctlpt( irit.E2, 0.5027, (-0.29742 ) ), \
                                                 irit.ctlpt( irit.E2, 0.52969, (-0.26192 ) ), \
                                                 irit.ctlpt( irit.E2, 0.56151, (-0.21786 ) ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 3, irit.list( \
                                                 irit.ctlpt( irit.E2, 0.56151, (-0.21786 ) ), \
                                                 irit.ctlpt( irit.E2, 0.54844, (-0.25493 ) ), \
                                                 irit.ctlpt( irit.E2, 0.53534, (-0.292 ) ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 5, irit.list( \
                                                 irit.ctlpt( irit.E2, 0.53534, (-0.292 ) ), \
                                                 irit.ctlpt( irit.E2, 0.50686, (-0.33034 ) ), \
                                                 irit.ctlpt( irit.E2, 0.48147, (-0.36179 ) ), \
                                                 irit.ctlpt( irit.E2, 0.45881, (-0.38634 ) ), \
                                                 irit.ctlpt( irit.E2, 0.43846, (-0.404 ) ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 5, irit.list( \
                                                 irit.ctlpt( irit.E2, 0.43846, (-0.404 ) ), \
                                                 irit.ctlpt( irit.E2, 0.4181, (-0.42165 ) ), \
                                                 irit.ctlpt( irit.E2, 0.39834, (-0.43373 ) ), \
                                                 irit.ctlpt( irit.E2, 0.37893, (-0.44024 ) ), \
                                                 irit.ctlpt( irit.E2, 0.35938, (-0.44117 ) ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 5, irit.list( \
                                                 irit.ctlpt( irit.E2, 0.35938, (-0.44117 ) ), \
                                                 irit.ctlpt( irit.E2, 0.33366, (-0.4424 ) ), \
                                                 irit.ctlpt( irit.E2, 0.31227, (-0.43435 ) ), \
                                                 irit.ctlpt( irit.E2, 0.29409, (-0.41701 ) ), \
                                                 irit.ctlpt( irit.E2, 0.27845, (-0.39038 ) ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 5, irit.list( \
                                                 irit.ctlpt( irit.E2, 0.27845, (-0.39038 ) ), \
                                                 irit.ctlpt( irit.E2, 0.26282, (-0.36375 ) ), \
                                                 irit.ctlpt( irit.E2, 0.25068, (-0.32893 ) ), \
                                                 irit.ctlpt( irit.E2, 0.24165, (-0.28593 ) ), \
                                                 irit.ctlpt( irit.E2, 0.23601, (-0.23475 ) ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 5, irit.list( \
                                                 irit.ctlpt( irit.E2, 0.23601, (-0.23475 ) ), \
                                                 irit.ctlpt( irit.E2, 0.23037, (-0.18356 ) ), \
                                                 irit.ctlpt( irit.E2, 0.228, (-0.12998 ) ), \
                                                 irit.ctlpt( irit.E2, 0.22966, (-0.07401 ) ), \
                                                 irit.ctlpt( irit.E2, 0.23632, (-0.01566 ) ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 5, irit.list( \
                                                 irit.ctlpt( irit.E2, 0.23632, (-0.01566 ) ), \
                                                 irit.ctlpt( irit.E2, 0.24336, 0.04587 ), \
                                                 irit.ctlpt( irit.E2, 0.25542, 0.10547 ), \
                                                 irit.ctlpt( irit.E2, 0.27369, 0.16314 ), \
                                                 irit.ctlpt( irit.E2, 0.29913, 0.21887 ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 5, irit.list( \
                                                 irit.ctlpt( irit.E2, 0.29913, 0.21887 ), \
                                                 irit.ctlpt( irit.E2, 0.32456, 0.2746 ), \
                                                 irit.ctlpt( irit.E2, 0.35453, 0.3222 ), \
                                                 irit.ctlpt( irit.E2, 0.38934, 0.36169 ), \
                                                 irit.ctlpt( irit.E2, 0.42865, 0.39305 ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 5, irit.list( \
                                                 irit.ctlpt( irit.E2, 0.42865, 0.39305 ), \
                                                 irit.ctlpt( irit.E2, 0.46795, 0.42441 ), \
                                                 irit.ctlpt( irit.E2, 0.50816, 0.44566 ), \
                                                 irit.ctlpt( irit.E2, 0.54845, 0.4568 ), \
                                                 irit.ctlpt( irit.E2, 0.58773, 0.45783 ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 5, irit.list( \
                                                 irit.ctlpt( irit.E2, 0.58773, 0.45783 ), \
                                                 irit.ctlpt( irit.E2, 0.61668, 0.45859 ), \
                                                 irit.ctlpt( irit.E2, 0.64512, 0.45283 ), \
                                                 irit.ctlpt( irit.E2, 0.67252, 0.44054 ), \
                                                 irit.ctlpt( irit.E2, 0.69823, 0.42175 ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 5, irit.list( \
                                                 irit.ctlpt( irit.E2, 0.69823, 0.42175 ), \
                                                 irit.ctlpt( irit.E2, 0.7131, 0.41083 ), \
                                                 irit.ctlpt( irit.E2, 0.72443, 0.4036 ), \
                                                 irit.ctlpt( irit.E2, 0.73253, 0.40005 ), \
                                                 irit.ctlpt( irit.E2, 0.73747, 0.40018 ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 5, irit.list( \
                                                 irit.ctlpt( irit.E2, 0.73747, 0.40018 ), \
                                                 irit.ctlpt( irit.E2, 0.74379, 0.40035 ), \
                                                 irit.ctlpt( irit.E2, 0.75037, 0.40299 ), \
                                                 irit.ctlpt( irit.E2, 0.75724, 0.40809 ), \
                                                 irit.ctlpt( irit.E2, 0.76443, 0.41566 ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 5, irit.list( \
                                                 irit.ctlpt( irit.E2, 0.76443, 0.41566 ), \
                                                 irit.ctlpt( irit.E2, 0.77162, 0.42324 ), \
                                                 irit.ctlpt( irit.E2, 0.77873, 0.43368 ), \
                                                 irit.ctlpt( irit.E2, 0.78579, 0.44701 ), \
                                                 irit.ctlpt( irit.E2, 0.79275, 0.4632 ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 3, irit.list( \
                                                 irit.ctlpt( irit.E2, 0.79275, 0.4632 ), \
                                                 irit.ctlpt( irit.E2, 0.80069, 0.46341 ), \
                                                 irit.ctlpt( irit.E2, 0.80862, 0.46361 ) ), irit.list( irit.KV_OPEN ) ) )

cnc = irit.list( cnc_c1, cnc_n2, cnc_c3 )
irit.free( cnc_c1 )
irit.free( cnc_n2 )
irit.free( cnc_c3 )

