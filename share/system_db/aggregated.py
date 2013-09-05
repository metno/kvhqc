# -*- coding: utf-8 -*- 

# run
#    select distinct paramid from data where typeid < 0 order by paramid
# to identify which parameters are aggregated, then consult parameter list to
# determine what they could be aggregated from

def add_agg(cur, p_from, p_to):
    cur.execute("INSERT INTO param_aggregated VALUES (?, ?)", (p_from, p_to))

def update(con, cur):
    cur.execute("DROP TABLE IF EXISTS param_aggregated")
    cur.execute("""CREATE TABLE IF NOT EXISTS param_aggregated (
           paramid_from INTEGER NOT NULL,
           paramid_to   INTEGER NOT NULL
    )""")

    ## aggregated preciptation
    RR_01, RR_1, RR_12, RR_24 = 105, 106, 109, 110
    add_agg(cur, RR_01, RR_1);
    add_agg(cur, RR_01, RR_12);
    add_agg(cur, RR_01, RR_24);
    add_agg(cur, RR_1, RR_12);
    add_agg(cur, RR_1, RR_24);
    add_agg(cur, RR_12, RR_24);

    ## aggregared sunshine time
    OT_1, OT_24 = 121, 122
    add_agg(cur, OT_1, OT_24);

    ## temperatures
    TA, TAM_24 = 211, 253
    TAN, TAN_12, TAN_24 = 213, 214, 251
    TAX, TAX_12, TAX_24 = 215, 216, 252
    TGN, TGN_12 = 223, 224
    add_agg(cur, TAN, TAN_12);
    add_agg(cur, TAN, TAN_24);
    add_agg(cur, TAX, TAX_12);
    add_agg(cur, TAX, TAX_24);
    add_agg(cur, TGN, TGN_12);
    add_agg(cur, TA,  TAM_24);

    ## humidity
    UU, UUM_24 = 262, 266
    add_agg(cur, UU, UUM_24);
