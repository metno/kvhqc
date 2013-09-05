# -*- coding: utf-8 -*- 

def update(con, cur):
    cur.execute("DROP TABLE IF EXISTS param_order")
    cur.execute("DROP TABLE IF EXISTS param_group_labels")
    cur.execute("DROP TABLE IF EXISTS param_groups")

    cur.execute("""CREATE TABLE param_groups (
           id INTEGER UNIQUE,
           sortkey INTEGER
    )""")
    cur.execute("""CREATE TABLE param_group_labels (
           group_id INTEGER NOT NULL REFERENCES param_groups(id),
           language CHAR(2) DEFAULT 'nb',
           label VARCHAR(32)
    )""")
    cur.execute("""CREATE TABLE param_order (
           paramid INTEGER NOT NULL,
           group_id INTEGER NOT NULL REFERENCES param_groups(id),
           sortkey INTEGER
    )""")

    paramorder_data = [
        # klstat
        ((211, 213, 215, 214, 216, 253, 242, 217, 262, 266, 173, 178, 61, 81, 86, 87, 90, 15, 16, 14, 55, 273, 41,
          42, 43, 31, 32, 33, 34, 36, 38, 40, 104, 109, 110, 112, 18, 7), u'Daglig rutine'),

        # airpress
        ((61, 81, 87, 90, 88, 91, 89, 92, 64, 85, 67, 86, 63, 83, 178, 173, 1, 177, 211, 175, 176, 174, 172), u'Lufttrykk'),

        # temperature
        ((211, 214, 216, 251, 252, 213, 215, 224, 242, 15, 14, 55, 41, 42, 43, 273, 262, 106,
          212, 221, 222, 223, 225, 217, 264, 265, 263, 244, 245, 243, 217, 121, 122), u'Temperatur'),

        # prec
        ((105, 104, 123, 106, 108, 109, 110, 1021, 1022, 112, 18, 7, 41, 42, 43, 31, 32, 33, 34, 36, 38, 40, 35, 37,
          39, 44, 45, 46, 262, 217, 211, 15, 55, 14, 23, 24, 22, 273, 121, 122), u'Nedbør og snøforhold'),

        # visual
        ((15, 55, 14, 23, 24, 22, 41, 42, 43, 44, 45, 46, 31, 32, 33, 34, 36, 38, 40,
          35, 37, 39, 273, 211, 214, 216, 262, 108, 109, 110, 105, 106, 123, 124, 117, 122), u'Visuell'),

        # wave
        ((211, 242, 217, 262, 173, 178, 177, 1, 61, 81, 86, 83, 87, 90, 15, 14, 55, 273, 41, 42, 43, 49, 47, 48, 23, 24,
          22, 131, 132, 133, 134, 151, 152, 153, 154, 65, 66, 21, 11, 401, 402, 403, 404, 9, 10), u'Maritime parametere'),

        # synop
        ((211, 214, 216, 213, 215, 262, 217, 242, 178, 1, 177, 61, 81, 86, 83, 19, 15, 14, 55,
          106, 108, 109, 110, 112, 18, 7, 273, 41, 42, 43, 23, 24, 22, 403, 404, 131, 134, 151, 154), u'Synop'),

        # priority
        ((211, 213, 215, 214, 216, 109, 110, 112, 7, 18, 15, 262, 173, 178, 41, 31, 32, 33, 42, 43, 34, 36,
          38, 40, 42, 43, 15, 273, 55, 242, 217), u'Prioriterte parametere'),

        # wind
        ((61, 81, 86, 88), u'Vind'),

        # plu
        ((105, 106, 109, 110, 104, 211), u'Pluviometerkontroll')
    ]

    for gid, (pids, label_nb) in enumerate(paramorder_data):
        cur.execute("INSERT INTO param_groups VALUES (?, ?)", (gid, gid))
        cur.execute("INSERT INTO param_group_labels (group_id, label) VALUES (?, ?)", (gid, label_nb))
        for psort, pid in enumerate(pids):
            cur.execute("INSERT INTO param_order VALUES (?, ?, ?)", (pid, gid, psort))
