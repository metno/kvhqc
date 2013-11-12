# -*- coding: utf-8 -*- 

def update(sdb):
    cur = sdb.cur
    cur.execute("DROP TABLE IF EXISTS param_related")

    cur.execute("""CREATE TABLE param_related (
           paramid INTEGER PRIMARY KEY,
           view_types_excluded VARCHAR(128) DEFAULT NULL,
           groupid INTEGER NOT NULL,
           sortkey INTEGER NOT NULL
    )""")

    
    related_params = [
        ( "RR_24", "V4", "V5", "V6", "SD", "SA", "EE" ),
        ( "TA", "TAN", "TAX" ),
        ( "UU", "TD" ),
        ( "PR", "PP", "PO", "PON", "POX" ),
        ( "FF", "FG", "FX", "FG_1", "FX_1" ),
        ( "DA",    "FA" ),
        #( "DD",    "FF" ),
        ( "DD_01", "FF_01" ),
        ( "DD_02", "FF_02" ),
        ( "DDM",   "FM" ),
        #( "DG",    "FG" ),
        ( "DG_010","FG_010" ),
        #( "DG_1",  "FG_1" ),
        ( "DG_6",  "FG_6" ),
        ( "DG_12", "FG_12" ),
        #( "DN",    "FN" ),
        #( "DG_02", "FN_02" ),
        ( "DW1",   "HW1" ),
        ( "DW2",   "HW2" ),
        #( "DX",    "FX" ),
        #( "DX_010","FX_010" ),
        #( "DX_1",  "FX_1" ),
        ( "DX_3",  "FX_3" ),
        ( "DX_6",  "FX_6" ),
        ( "DX_12", "FX_12" ),
        ( "CC1",   "HS1", "NS1" ),
        ( "CC2",   "HS2", "NS2" ),
        ( "CC3",   "HS3", "NS3" ),
        ( "CC4",   "HS4", "NS4" ),
        ]

    groupid = 0
    for rp in related_params:
        groupid += 1
        
        for sk, r in enumerate(rp):
            pid = sdb.paramid(r)
            try:
                cur.execute("INSERT INTO param_related (paramid, groupid, sortkey) VALUES (?, ?, ?)", (pid, groupid, sk))
            except:
                print "db error while inserting param %s for group %s" % (r, (',').join(rp))
                raise

    cur.execute("UPDATE param_related SET view_types_excluded = 'TimeSeries' WHERE paramid = ?", (sdb.paramid('PP'), ))
