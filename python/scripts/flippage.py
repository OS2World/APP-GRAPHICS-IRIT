#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  A simple simulation of page flipping.
# 
#                                Gershon Elber, March 1994.
# 

page1 = irit.ruledsrf( irit.ctlpt( irit.E2, 0, 0 ) + \
                       irit.ctlpt( irit.E2, 1, 0 ), \
                       irit.ctlpt( irit.E2, 0, 1.5 ) + \
                       irit.ctlpt( irit.E2, 1, 1.5 ) )
page1 = irit.sraise( irit.sraise( page1, irit.ROW, 4 ), irit.COL, 4 )
irit.color( page1, irit.GREEN )
irit.viewobj( page1 )

page2 = irit.seditpt( irit.coerce( page1, irit.E3 ), irit.ctlpt( irit.E3, 0.9, 1.35, 0.5 ), 3, 3 )
irit.color( page2, irit.RED )
irit.viewobj( page2 )

page3 = irit.srefine( page2, irit.COL, 0, irit.list( 0.3, 0.6 ) )
page3 = irit.seditpt( irit.coerce( page3, irit.E3 ), irit.ctlpt( irit.E3, 0.6, 1.5, 0.4 ), 5, 3 )
page3 = irit.seditpt( irit.coerce( page3, irit.E3 ), irit.ctlpt( irit.E3, 0.6, 1, 0.4 ), 5, 2 )
page3 = irit.seditpt( irit.coerce( page3, irit.E3 ), irit.ctlpt( irit.E3, 0.6, 0.5, 0.4 ), 5, 1 )
page3 = irit.seditpt( irit.coerce( page3, irit.E3 ), irit.ctlpt( irit.E3, 0.6, 0, 0.4 ), 5, 0 )
irit.color( page3, irit.CYAN )
irit.viewobj( page3 )

page4 = irit.srefine( page2, irit.COL, 0, irit.list( 0.3 ) )
page4 = irit.seditpt( irit.coerce( page4, irit.E3 ), irit.ctlpt( irit.E3, 0, 1.5, 0.4 ), 4, 3 )
page4 = irit.seditpt( irit.coerce( page4, irit.E3 ), irit.ctlpt( irit.E3, 0, 1, 0.4 ), 4, 2 )
page4 = irit.seditpt( irit.coerce( page4, irit.E3 ), irit.ctlpt( irit.E3, 0, 0.5, 0.4 ), 4, 1 )
page4 = irit.seditpt( irit.coerce( page4, irit.E3 ), irit.ctlpt( irit.E3, 0, 0, 0.4 ), 4, 0 )
page4 = irit.seditpt( irit.coerce( page4, irit.E3 ), irit.ctlpt( irit.E3, 0.5, 1.5, 0.4 ), 3, 3 )
page4 = irit.seditpt( irit.coerce( page4, irit.E3 ), irit.ctlpt( irit.E3, 0.5, 1, 0.4 ), 3, 2 )
page4 = irit.seditpt( irit.coerce( page4, irit.E3 ), irit.ctlpt( irit.E3, 0.5, 0.5, 0.4 ), 3, 1 )
page4 = irit.seditpt( irit.coerce( page4, irit.E3 ), irit.ctlpt( irit.E3, 0.5, 0, 0.4 ), 3, 0 )
irit.color( page4, irit.MAGENTA )
irit.viewobj( page4 )

page5 = irit.srefine( page2, irit.COL, 0, irit.list( 0.3 ) )
page5 = irit.seditpt( irit.coerce( page5, irit.E3 ), irit.ctlpt( irit.E3, (-0.5 ), 1.5, 0.1 ), 4, 3 )
page5 = irit.seditpt( irit.coerce( page5, irit.E3 ), irit.ctlpt( irit.E3, (-0.5 ), 1, 0.1 ), 4, 2 )
page5 = irit.seditpt( irit.coerce( page5, irit.E3 ), irit.ctlpt( irit.E3, (-0.5 ), 0.5, 0.1 ), 4, 1 )
page5 = irit.seditpt( irit.coerce( page5, irit.E3 ), irit.ctlpt( irit.E3, (-0.5 ), 0, 0.1 ), 4, 0 )
page5 = irit.seditpt( irit.coerce( page5, irit.E3 ), irit.ctlpt( irit.E3, (-0.2 ), 1.5, 0.1 ), 3, 3 )
page5 = irit.seditpt( irit.coerce( page5, irit.E3 ), irit.ctlpt( irit.E3, (-0.2 ), 1, 0.1 ), 3, 2 )
page5 = irit.seditpt( irit.coerce( page5, irit.E3 ), irit.ctlpt( irit.E3, (-0.2 ), 0.5, 0.1 ), 3, 1 )
page5 = irit.seditpt( irit.coerce( page5, irit.E3 ), irit.ctlpt( irit.E3, (-0.2 ), 0, 0.1 ), 3, 0 )
page5 = irit.seditpt( irit.coerce( page5, irit.E3 ), irit.ctlpt( irit.E3, 0.2, 1.5, 0.4 ), 2, 3 )
page5 = irit.seditpt( irit.coerce( page5, irit.E3 ), irit.ctlpt( irit.E3, 0.2, 1, 0.4 ), 2, 2 )
page5 = irit.seditpt( irit.coerce( page5, irit.E3 ), irit.ctlpt( irit.E3, 0.2, 0.5, 0.4 ), 2, 1 )
page5 = irit.seditpt( irit.coerce( page5, irit.E3 ), irit.ctlpt( irit.E3, 0.2, 0, 0.4 ), 2, 0 )
page5 = irit.seditpt( irit.coerce( page5, irit.E3 ), irit.ctlpt( irit.E3, 0.2, 1.5, 0 ), 1, 3 )
page5 = irit.seditpt( irit.coerce( page5, irit.E3 ), irit.ctlpt( irit.E3, 0.2, 1, 0 ), 1, 2 )
page5 = irit.seditpt( irit.coerce( page5, irit.E3 ), irit.ctlpt( irit.E3, 0.2, 0.5, 0 ), 1, 1 )
page5 = irit.seditpt( irit.coerce( page5, irit.E3 ), irit.ctlpt( irit.E3, 0.2, 0, 0 ), 1, 0 )
irit.color( page5, irit.YELLOW )
irit.viewobj( page5 )


page6 = page1 * irit.ry( 180 )
irit.color( page6, irit.WHITE )
irit.viewobj( page6 )


pages = irit.list( page1, page2, page3, page4, page5, page6 )
irit.view( pages, irit.ON )
irit.free( pages )

srfmorphlist = irit.nil(  )

def morphpage( srf1, srf2, step ):
    irit.ffcompat( srf1, srf2 )
    i = 0
    while ( i <= 1 ):
        irit.snoc( irit.smorph( srf1, srf2, i ), srfmorphlist )
        i = i + step

morphpage( page1, page2, 0.1 )
morphpage( page2, page3, 0.1 )
morphpage( page3, page4, 0.1 )
morphpage( page4, page5, 0.1 )
morphpage( page5, page6, 0.1 )

i = 1
while ( i <= irit.SizeOf( srfmorphlist ) ):
    irit.view( irit.nth( srfmorphlist, i ), irit.ON )
    i = i + 1
irit.free( srfmorphlist )

tv1 = irit.tfromsrfs( irit.list( page1, page2, page3, page4, page5, page6 ),\
3, irit.KV_OPEN )
irit.interact( tv1 )
wmin = irit.FetchRealObject(irit.nth( irit.pdomain( tv1 ), 5 ))
wmax = irit.FetchRealObject(irit.nth( irit.pdomain( tv1 ), 6 ))
i = 0
while ( i <= 100 ):
    irit.view( irit.strivar( tv1, irit.DEPTH, ( wmin * ( 100 - i ) + wmax * i )/100 ), irit.ON )
    i = i + 1

tv1 = irit.tfromsrfs( irit.list( page1, page2, page3, page4, page5, page6 ),\
4, irit.KV_OPEN )
irit.interact( tv1 )
wmin = irit.FetchRealObject(irit.nth( irit.pdomain( tv1 ), 5 ))
wmax = irit.FetchRealObject(irit.nth( irit.pdomain( tv1 ), 6 ))
i = 0
while ( i <= 100 ):
    irit.view( irit.strivar( tv1, irit.DEPTH, ( wmin * ( 100 - i ) + wmax * i )/100 ), irit.ON )
    i = i + 1

tv1 = irit.tfromsrfs( irit.list( page1, page2, page3, page4, page5, page6 ),\
5, irit.KV_OPEN )
irit.interact( tv1 )
wmin = irit.FetchRealObject(irit.nth( irit.pdomain( tv1 ), 5 ))
wmax = irit.FetchRealObject(irit.nth( irit.pdomain( tv1 ), 6 ))
i = 0
while ( i <= 100 ):
    irit.view( irit.strivar( tv1, irit.DEPTH, ( wmin * ( 100 - i ) + wmax * i )/100 ), irit.ON )
    i = i + 1

tv1 = irit.tfromsrfs( irit.list( page1, page2, page3, page4, page5, page6 ),\
6, irit.KV_OPEN )
irit.interact( tv1 )
wmin = irit.FetchRealObject(irit.nth( irit.pdomain( tv1 ), 5 ))
wmax = irit.FetchRealObject(irit.nth( irit.pdomain( tv1 ), 6 ))
i = 0
while ( i <= 100 ):
    irit.view( irit.strivar( tv1, irit.DEPTH, ( wmin * ( 100 - i ) + wmax * i )/100 ), irit.ON )
    i = i + 1

i = 0
while ( i <= 11 ):
    irit.save( "page" + str(i), irit.strivar( tv1, irit.DEPTH, ( wmin * ( 11 - i ) + wmax * i )/11 ) )
    i = i + 1

#  for( i = 0, 1, 11,
#      save( "pageo" +i ,
#           offset( strivar( Tv1, depth, (wmin * (11 - i) + wmax * i) / 11.0 ),
#                   0.003, 0.0003, false ) ) );

irit.free( page1 )
irit.free( page2 )
irit.free( page3 )
irit.free( page4 )
irit.free( page5 )
irit.free( page6 )

irit.free( tv1 )

