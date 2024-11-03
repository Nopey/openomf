macro(openomf_strings)
    # strings new in OpenOMF
    openomf_string(LANG2_STR_LANGUAGE "Language name")
endmacro()

macro(base_strings)
    # strings from OMF 2097
    # from_base(OMF2097 string idx, LANG2_STR_ ENUM NAME, Description)
    from_base(LANG2_STR_STADIUMNAME "Stadium Names" 56..=60)
    from_base(LANG2_STR_PRONOUN "Newsroom Pronouns" 81..=86)
    from_base(LANG2_STR_NEWSROOM_UNRANKEDCHALLENGER_DIALOG "Newsroom Unrnkd Chlngr Dialog" 79)
    from_base(LANG2_STR_NEWSROOM_UNRANKEDCHALLENGER "Newsroom Unranked Challenger" 77..=78)
    from_base(LANG2_STR_NEWSROOM_NEWCHAMPION "Newsroom New Champion" 79)
    from_base(LANG2_STR_NEWSROOM_REPORT "Newsroom Reports" 87..=134)
endmacro()
