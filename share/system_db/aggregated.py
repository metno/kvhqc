# -*- coding: utf-8 -*- 

# run
#    select distinct paramid from data where typeid < 0 order by paramid
# to identify which parameters are aggregated, then consult parameter list to
# determine what they could be aggregated from

def add_agg(sdb, pn_from, pn_to):
    p_from = sdb.paramid(pn_from)
    p_to   = sdb.paramid(pn_to)
    sdb.cur.execute("INSERT INTO param_aggregated VALUES (?, ?)", (p_from, p_to))

def update(sdb):
    cur = sdb.cur
    cur.execute("DROP TABLE IF EXISTS param_aggregated")
    cur.execute("""CREATE TABLE IF NOT EXISTS param_aggregated (
           paramid_from INTEGER NOT NULL,
           paramid_to   INTEGER NOT NULL
    )""")

    ## aggregated preciptation
    add_agg(sdb, "RR_01", "RR_1");
    add_agg(sdb, "RR_01", "RR_12");
    add_agg(sdb, "RR_01", "RR_24");
    add_agg(sdb, "RR_1", "RR_12");
    add_agg(sdb, "RR_1", "RR_24");
    add_agg(sdb, "RR_12", "RR_24");

    ## aggregared sunshine time
    add_agg(sdb, "OT_1", "OT_24");

    ## temperatures
    add_agg(sdb, "TAN", "TAN_12");
    add_agg(sdb, "TAN", "TAN_24");
    add_agg(sdb, "TAX", "TAX_12");
    add_agg(sdb, "TAX", "TAX_24");
    add_agg(sdb, "TGN", "TGN_12");
    add_agg(sdb, "TA",  "TAM_24");

    ## humidity
    add_agg(sdb, "UU", "UUM_24");
