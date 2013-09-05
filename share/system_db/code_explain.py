# -*- coding: utf-8 -*- 

def update(con, cur):
    cur.execute("DROP TABLE IF EXISTS code_short")
    cur.execute("""CREATE TABLE IF NOT EXISTS code_short (
           paramid INTEGER NOT NULL,
           code INTEGER NOT NULL,
           language CHAR(2) NOT NULL,
           description VARCHAR(16)
    )""")

    cur.execute("DROP TABLE IF EXISTS code_explain")
    cur.execute("""CREATE TABLE IF NOT EXISTS code_explain (
           paramid INTEGER NOT NULL,
           code INTEGER NOT NULL,
           language CHAR(2) NOT NULL,
           description VARCHAR(1024)
    )""")

    # explanations for WW = WMO code 4677
    explain_WW_en = [ 
        [  0, u"Cloud development not observed or not observable" ],
        [  1, u"Cloud generally dissolving or becoming less developed" ],
        [  2, u"State of sky on the whole unchanged" ],
        [  3, u"Clouds generally forming or developing" ],
        [  4, u"Visibility reduced by smoke, e.g. veldt or forest fires, industrial smoke or volcanic ashes" ],
        [  5, u"Haze" ],
        [  6, u"Widespread dust in suspension in the air, not raised by wind at or near the station at the time of observation" ],
        [  7, u"Dust or sand raised by wind at or near the station at the time of"
          + u" observation, but not well-developed dust whirl(s) or sand whirl(s)"
          + u" and no duststorm or sandstorm seen; or, in the case of ships, blowing spray at the station" ],
        [  8, u"Well-developed dust or sand whirl(s) seen at or near the station during the"
          + u" preceding hour or at the time of observation, but no dust storm or sandstorm" ],
        [  9, u"Duststorm or sandstorm within sight at the time of observation, or at the station during the preceding hour" ],
        [ 10, u"Mist" ],
        [ 11, u"Patches of shallow fog or ice fog at the station, whether on land or sea not"
          + u" deeper than about 2 metres on land or 10 metres at sea" ],
        [ 12, u"More or less continuous shallow fog or ice fog at the station, whether on"
          + u" land or sea, not deeper than about 2m/land or 10m/sea" ],
        [ 13, u"Lightning visible, or thunder heard" ],
        [ 14, u"Precipitation within sight, not reaching the ground or the surface of the sea" ],
        [ 15, u"Precipitation within sight, reaching the ground or the surface of the sea,"
          + u" but distant, i.e. > 5 km from the station" ],
        [ 16, u"Precipitation within sight, reaching the ground or the surface of the sea,"
          + u" near to, but not at the station" ],
        [ 17, u"Thunderstorm, but no precipitation at the time of observation" ],
        [ 18, u"Squalls at or within sight of the station during the preceding hour or at the time of observation" ],
        [ 19, u"Funnel clouds at or within sight of the station during the preceding hour or at the time of observation" ],
        [ 20, u"Drizzle (not freezing) or snow grains, not falling as showers, during the"
          + u" preceding hour but not at the time of observation" ],
        [ 21, u"Rain (not freezing), not falling as showers, during the preceding hour but not at the time of observation" ],
        [ 22, u"Snow, not falling as showers, during the preceding hour but not at the time of observation" ],
        [ 23, u"Rain and snow or ice pellets, not falling as showers; during the preceding"
          + u" hour but not at the time of observation" ],
        [ 24, u"Freezing drizzle or freezing rain; during the preceding hour but not at the time of observation" ],
        [ 25, u"Shower(s) of rain during the preceding hour but not at the time of observation" ],
        [ 26, u"Shower(s) of snow, or of rain and snow during the preceding hour but not at the time of observation" ],
        [ 27, u"Shower(s) of hail, or of rain and hail during the preceding hour but not at the time of observation" ],
        [ 28, u"Fog or ice fog during the preceding hour but not at the time of observation" ],
        [ 29, u"Thunderstorm (with or without precipitation) during the preceding hour but not at the time of observation" ],
        [ 30, u"Slight or moderate duststorm or sandstorm - has decreased during the preceding hour" ],
        [ 31, u"Slight or moderate duststorm or sandstorm - no appreciable change during the preceding hour" ],
        [ 32, u"Slight or moderate duststorm or sandstorm - has begun or has increased during the preceding hour" ],
        [ 33, u"Severe duststorm or sandstorm - has decreased during the preceding hour" ],
        [ 34, u"Severe duststorm or sandstorm - no appreciable change during the preceding hour" ],
        [ 35, u"Severe duststorm or sandstorm - has begun or has increased during the preceding hour" ],
        [ 36, u"Slight/moderate drifting snow - generally low (below eye level)" ],
        [ 37, u"Heavy drifting snow - generally low (below eye level)" ],
        [ 38, u"Slight/moderate blowing snow - generally high (above eye level)" ],
        [ 39, u"Heavy blowing snow - generally high (above eye level)" ],
        [ 40, u"Fog or ice fog at a a distance at the time of observation, but not at"
          + u" station during the preceding hour, the fog or ice fog extending to a level above that of  the observer" ],
        [ 41, u"Fog or ice fog in patches" ],
        [ 42, u"Fog/ice fog, sky visible, has become thinner during the preceding hour" ],
        [ 43, u"Fog/ice fog, sky invisible, has become thinner during the preceding hour" ],
        [ 44, u"Fog or ice fog, sky visible, no appreciable change during the past hour" ],
        [ 45, u"Fog or ice fog, sky invisible, no appreciable change during the preceding hour" ],
        [ 46, u"Fog or ice fog, sky visible, has begun or has become thicker during preceding hour" ],
        [ 47, u"Fog or ice fog, sky invisible, has begun or has become thicker during the preceding hour" ],
        [ 48, u"Fog, depositing rime, sky visible" ],
        [ 49, u"Fog, depositing rime, sky invisible" ],
        [ 50, u"Drizzle, not freezing, intermittent, slight at time of ob." ],              
        [ 51, u"Drizzle, not freezing, continuous, slight at time of ob." ],              
        [ 52, u"Drizzle, not freezing, intermittent, moderate at time of ob." ],           
        [ 53, u"Drizzle, not freezing, continuous, moderate at time of ob." ],             
        [ 54, u"Drizzle, not freezing, intermittent, heavy at time of ob." ],              
        [ 55, u"Drizzle, not freezing, continuous, heavy at time of ob." ],                
        [ 56, u"Drizzle, freezing, slight" ],
        [ 57, u"Drizzle, freezing, moderate or heavy (dense)" ],
        [ 58, u"Rain and drizzle, slight" ],
        [ 59, u"Rain and drizzle, moderate or heavy" ],
        [ 60, u"Rain, not freezing, intermittent, slight at time of ob." ],                
        [ 61, u"Rain, not freezing, continuous, slight at time of ob." ],                  
        [ 62, u"Rain, not freezing, intermittent, moderate at time of ob." ],              
        [ 63, u"Rain, not freezing, continuous, moderate at time of ob." ],                
        [ 64, u"Rain, not freezing, intermittent, heavy at time of ob." ],                 
        [ 65, u"Rain, not freezing, continuous, heavy at time of ob." ],                   
        [ 66, u"Rain, freezing, slight" ],
        [ 67, u"Rain, freezing, moderate or heavy" ],
        [ 68, u"Rain or drizzle and snow, slight" ],
        [ 69, u"Rain or drizzle and snow, moderate or heavy" ],
        [ 70, u"Intermittent fall of snowflakes, slight at time of ob." ],                 
        [ 71, u"Continuous fall of snowflakes, slight at time of ob." ],                   
        [ 72, u"Intermittent fall of snowflakes, moderate at time of ob." ],               
        [ 73, u"Continuous fall of snowflakes, moderate at time of ob." ],                 
        [ 74, u"Intermittent fall of snowflakes, heavy at time of ob." ],                  
        [ 75, u"Continuous fall of snowflakes, heavy at time of ob." ],                    
        [ 76, u"Diamond dust (with or without fog)" ],
        [ 77, u"Snow grains (with or without fog)" ],
        [ 78, u"Isolated star-like snow crystals (with or without fog)" ],
        [ 79, u"Ice pellets" ],
        [ 80, u"Rain shower(s), slight" ],
        [ 81, u"Rain shower(s), moderate or heavy" ],
        [ 82, u"Rain shower(s), violent" ],
        [ 83, u"Shower(s) of rain and snow, slight" ],
        [ 84, u"Shower(s) of rain and snow, moderate or heavy" ],
        [ 85, u"Snow shower(s), slight" ],
        [ 86, u"Snow shower(s), moderate or heavy" ],
        [ 87, u"Shower(s) of snow pellets or small hail, with or without rain or rain and snow mixed - slight" ],
        [ 88, u"Shower(s) of snow pellets or small hail, with or without rain or rain and snow mixed - moderate or heavy" ],
        [ 89, u"Shower(s) of hail, with or without rain or rain and snow mixed, not associated with thunder - slight" ],
        [ 90, u"Shower(s) of hail, with or without rain or rain and snow mixed, not associated with thunder - moderate or heavy" ],
        [ 91, u"Slight rain at time of observation - Thunderstorm during the preceding hour but not at time of observation" ],
        [ 92, u"Moderate or heavy rain at time of observation - Thunderstorm during the preceding hour but not at time of observation" ],
        [ 93, u"Slight snow, or rain and snow mixed or hail at time of observation -"
          + " Thunderstorm during the preceding hour but not at time of observation" ],
        [ 94, u"Moderate or heavy snow, or rain and snow mixed or hail at time of"
          + " observation - Thunderstorm during the preceding hour but not at time of observation" ],
        [ 95, u"Thunderstorm, slight or moderate, without hail, but with rain and/or snow at time of observation" ],
        [ 96, u"Thunderstorm, slight or moderate, with hail at time of ob." ],             
        [ 97, u"Thunderstorm, heavy, without hail, but with rain and/or snow at time of observation" ],
        [ 98, u"Thunderstorm combined with dust/sandstorm at time of observation" ],
        [ 99, u"Thunderstorm, heavy with hail at time of observation" ]
    ]
    for ex in explain_WW_en:
        cur.execute("INSERT INTO code_explain VALUES (41, ?, 'en', ?)", ex)

    explain_Wx_en = [
        (0, u"cloud covering half or less of the sky throughout the period"),
        (1, u"cloud covering more than half the sky during part of the period & half or less for the rest"),
        (2, u"cloud covering more than half the sky throughout the period"),
        (3, u"sandstorm, duststorm or blowing snow"),
        (4, u"fog or ice fog or thick haze"),
        (5, u"drizzle"),
        (6, u"rain"),
        (7, u"snow, or rain and snow mixed"),
        (8, u"shower(s)"),
        (9, u"thunderstorm(s) with or without precipitation")
    ]
    for ex in explain_Wx_en:
        cur.execute("INSERT INTO code_explain VALUES (42, ?, 'en', ?)", ex)
        cur.execute("INSERT INTO code_explain VALUES (43, ?, 'en', ?)", ex)

    explain_RS_en = [
        [ 0, "Ice not building up" ],
        [ 1, "Ice building up slowly(0.6 sm/h and less)" ],
        [ 2, "Ice building up rapidly(0.7 sm/h and more)" ],
        [ 3, "Ice melting or breaking up slowly" ],
        [ 4, "Ice melting or breaking up rapidly" ]
    ]
    for ex in explain_RS_en:
        cur.execute("INSERT INTO code_explain VALUES (17, ?, 'en', ?)", ex)

    explain_CCx_en = [
        (0, u"cirrus  (CI)"),
        (1, u"cirricumulus (CC)"),
        (2, u"cirrostratus (CS)"),
        (3, u"altocumulus (AC)"),
        (4, u"altostratus (AS)"),
        (5, u"nimbostratus (NS)"),
        (6, u"stratocumulus (SC)"),
        (7, u"stratus (ST)"),
        (8, u"cumulus (CU)"),
        (9, u"cumulonimbus (CB)")
    ]
    for ex in explain_CCx_en:
        cur.execute("INSERT INTO code_explain VALUES (305, ?, 'en', ?)", ex)
        cur.execute("INSERT INTO code_explain VALUES (306, ?, 'en', ?)", ex)
        cur.execute("INSERT INTO code_explain VALUES (307, ?, 'en', ?)", ex)
        cur.execute("INSERT INTO code_explain VALUES (308, ?, 'en', ?)", ex)

    explain_Nx_en = [
        (-1, u"cloud is indiscernible for reasons other than fog or other meteorological phenomena, or observation is not made"),
        (0, u"sky clear"),
        (1, u"1 okta : 1/10 - 2/10"),
        (2, u"2 oktas : 2/10 - 3/10"),
        (3, u"3 oktas : 4/10"),
        (4, u"4 oktas : 5/10"),
        (5, u"5 oktas : 6/10"),
        (6, u"6 oktas : 7/10 - 8/10"),
        (7, u"7 oktas or more, but not 8 oktas : 9/10 or more, but not 10/10"),
        (8, u"8 oktas : 10/10"),
        (9, u"sky obscured by fog or other meteorological phenomena")
    ]
    for ex in explain_Nx_en:
        cur.execute("INSERT INTO code_explain VALUES (14, ?, 'en', ?)", ex)
        cur.execute("INSERT INTO code_explain VALUES (15, ?, 'en', ?)", ex)
        cur.execute("INSERT INTO code_explain VALUES (16, ?, 'en', ?)", ex)
