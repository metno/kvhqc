# -*- coding: utf-8 -*- 

def update(con, cur):
    cur.execute("DROP TABLE IF EXISTS slimits")

    cur.execute("""CREATE TABLE slimits (
           paramid INTEGER PRIMARY KEY,
           low FLOAT NOT NULL,
           high FLOAT NOT NULL
    )""")

    slimit_values = [
        (  1,    0,     9),
        (  2,    0,     9),
        (  3,    0,    60),
        (  4,    0,     9),
        (  6,    0,     9),
        (  7,   -1,     9),
        (  8,    0,    90),
        (  9,    1,     4),
        ( 10,    1,     7),
        ( 12,    0,     9),
        ( 13,    0,     9),
        ( 14,    0,     9),
        ( 15,    0,     9),
        ( 16,    0,     9),
        ( 17,    0,     9),
        ( 18,   -1,     9),
        ( 19,    0,     9),
        ( 20,   -1,     9),
        ( 21,    0,     9),
        ( 22,    0,     9),
        ( 23,    0,     9),
        ( 24,    0,     9),
        ( 25,    0,     9),
        ( 26,    0,     9),
        ( 27,    0,     9),
        ( 28,    0,     9),
        ( 31,    1,    29),
        ( 32,    1,    29),
        ( 33,    1,    29),
        ( 34,    1,    29),
        ( 35,    0,     2),
        ( 36,    1,    29),
        ( 37,    0,     2),
        ( 38,    1,    29),
        ( 39,    0,     2),
        ( 40,    1,    29),
        ( 41,    0,    99),
        ( 42,    0,     9),
        ( 43,    0,     9),
        ( 44,    0,     9),
        ( 45,    0,     9),
        ( 46,    0,     9),
        ( 54,   -1,     9),
        ( 55,   -3,  2500),
        ( 57,    0,  5000),
        ( 61,   -3,   360),
        ( 62,    0,   360),
        ( 63,    0,   360),
        ( 64,    0,   360),
        ( 65,    0,   360),
        ( 66,    0,   360),
        ( 67,    0,   360),
        ( 68,    0,   360),
        ( 81,    0,    98),
        ( 82,    0,    26),
        ( 83,    0,   131),
        ( 84,    0,   131),
        ( 85,    0,    92),
        ( 86,    0,    93),
        ( 87,    0,    93),
        ( 90,    0,    93),
        (101,    0,    10),
        (104,    0,   615),
        (105,   -6,     5),
        (106,   -1,    50),
        (107,   -1,    75),
        (108,   -1,   100),
        (109,   -1,   120),
        (110,   -1,   150),
        (111,    0,     5),
        (112,   -3,  1000),
        (113,    0,     5),
        (114,    0,     5),
        (117,   -1, 10000),
        (121,    0,    60),
        (122,    0,  1440),
        (123,    0,    60),
        (124,    0,  1440),
        (125,    0,    60),
        (126,    0,    60),
        (131,    0,    50),
        (132, -0.1,    40),
        (133, -0.1,    40),
        (134, -0.1,    40),
        (151,    0,    60),
        (152,    0,    60),
        (153,    0,    60),
        (154,    0,    60),
        (171,  800,  1100),
        (172,  899,  1063),
        (173,  800,  1100),
        (174,  800,  1100),
        (175,  800,  1100),
        (176,  800,  1100),
        (177,  -25,    16),
        (178,  851,  1069),
        (197, -100,  4500),
        (198,  -10,   500),
        (199,  -10,   500),
        (200,  -10,  1600),
        (201,  -10,  1600),
        (202,  -10,  1600),
        (211,  -55,    50),
        (212,  -53,    45),
        (213,  -55,    50),
        (214,  -59,    49),
        (215,  -55,    50),
        (216,  -52,    47),
        (217,  -55,    50),
        (221,  -56,    56),
        (222,  -56,    56),
        (223,  -56,    56),
        (224,  -56,    56),
        (225,  -56,    56),
    # 227 100 -55 50
    # 227 10 -13 29
    # 227 20 -55 50
    # 227 50 -55 50
        (227,  -55,    50),
        (228,  -55,    50),
        (229,  -55,    50),
        (234,  -55,    40),
        (242,   -2,    30),
        (243,   -2,    30),
        (246,  -55,    50),
        (253,  -55,    50),
        (261,    0,   130),
        (262,    0,   104),
        (263,    0,   104),
        (264,    0,   104),
        (265,    0,   104),
        (273,    0, 75001),
        (301,    0,    89),
        (302,    0,    89),
        (303,    0,    89),
        (304,    0,    89),
        (305,    0,     9),
        (306,    0,     9),
        (307,    0,     9),
        (308,    0,     9),
        (401,  -90,    90),
        (402, -180,   180),
        (403,    0,   360),
        (404,    0,    50)
    ]

    for sv in slimit_values:
        cur.execute("INSERT INTO slimits VALUES (?, ?, ?)", sv)