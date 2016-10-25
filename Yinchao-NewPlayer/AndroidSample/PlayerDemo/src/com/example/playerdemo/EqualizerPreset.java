package com.example.playerdemo;

import java.util.*;

/**
 * 音效相关操作
 *
 * @author hu.cao
 * @version 5.9.0
 */
public class EqualizerPreset {
    /**
     * 每种音效包含10个数据，即10跟柱子.
     */
    public static final short COUNT_EQU_BAND = 10;

    private static final Map<String, short[]> PRESET_EQUALIZER_MAP = new LinkedHashMap<String, short[]>();
    private static final List<String> PRESET_NAME_LIST = new ArrayList<String>();

    static final int CUSTOME_STYLE = 1000;
    static final String GENRE_CUSTOM = "自定义/custom";
    static final String GENRE_NORMAL = "普通/Normal";

    /*流行*/
    static final String GENRE_POP = "流行/Pop";
    static final String GENRE_RB = "R&B/R&B";
    static final String GENRE_HIP_HOP = "嘻哈/Hip-Hop";
    static final String GENRE_RAP = "说唱/Rap";
    static final String GENRE_BLUES = "布鲁斯/Blues";
    static final String GENRE_CHANSON = "法国香颂/Chanson";
    static final String GENRE_SOUL = "灵魂/Soul";
    static final String GENRE_REGGAE = "雷鬼/Reggae";
    static final String GENRE_PROGRESSIVE = "前卫/Progressive";
    static final String GENRE_GOSPEL = "福音/Gospel";
    static final String GENRE_LATIN_POP = "拉丁/Latin Pop";
    static final String GENRE_INDIE_POP = "独立流行/Indie Pop";
    static final String GENRE_NEW_MEDIA = "网络流行/New Media";
    static final String GENRE_ALTERNATIVE_POP = "另类流行/Alternative Pop";
    static final String GENRE_TEEN_POP = "Teen Pop/Teen-Pop";
    static final String GENRE_DANCE_POP = "Dance-Pop/Dance-Pop";
    static final String GENRE_J_POP = "J-POP/J-POP";
    static final String GENRE_K_POP = "K-POP/K-POP";
    static final String GENRE_DREAM_POP = "Dream-pop/Dream-pop";
    static final String GENRE_SYNTHPOP = "Synthpop/Synthpop";
    static final String GENRE_URBAN = "Urban/Urban";

    /*摇滚*/
    static final String GENRE_ROCK = "摇滚/Rock";
    static final String GENRE_POP_ROCK = "流行摇滚/Pop Rock";
    static final String GENRE_BRIT_POP = "英伦摇滚/Brit-pop";
    static final String GENRE_FOLK_ROCK = "民谣摇滚/FolkRock";
    static final String GENRE_COUNTRY_ROCK = "乡村摇滚/Country Rock";
    static final String GENRE_INDIE_ROCK = "独立摇滚/Indie Rock";
    static final String GENRE_ARTROCK = "艺术摇滚/ArtRock";
    static final String GENRE_HARD_ROCK = "硬摇滚/Hard Rock";
    static final String GENRE_SOFT_ROCK = "软摇滚/Soft Rock";
    static final String GENRE_ALTERNATIVE_ROCK = "另类摇滚/Alternative Rock";
    static final String GENRE_POSTROCK = "后摇/PostRock";
    static final String GENRE_HEAVY_METAL = "重金属/Heavy Metal";
    static final String GENRE_METAL = "金属/Metal";
    static final String GENRE_POP_METAL = "流行金属/Pop Metal";
    static final String GENRE_NU_METAL = "新金属/Nu-Metal";
    static final String GENRE_DEATH_METAL = "死亡金属/Death Metal";
    static final String GENRE_NEO_CLASSICAL_METAL = "新古典金属/Neo - Classical Metal";
    static final String GENRE_PROGRESSIVE_METAL = "前卫金属/Progressive Metal";
    static final String GENRE_INDUSTRIAL_METAL = "工业金属/Industrial Metal";
    static final String GENRE_BLACK_METAL = "黑金属/Black Metal";
    static final String GENRE_GOTHIC_METAL = "歌特金属/Gothic Metal";
    static final String GENRE_RAP_METAL = "说唱金属/Rap Metal";
    static final String GENRE_SYMPHONIC_METAL = "交响金属/Symphonic Metal";
    static final String GENRE_HARDCORE = "硬核/Hardcore";
    static final String GENRE_POST_HARDCORE = "后硬核/Post Hardcore";
    static final String GENRE_PUNK = "朋克/Punk";
    static final String GENRE_POST_PUNK = "后朋克/Post Punk";
    static final String GENRE_PUNK_POP = "流行朋克/Punk - Pop";
    static final String GENRE_POST_GRUNGE = "后垃圾/Post Grunge";
    static final String GENRE_PSYCHEDELIC = "迷幻/Psychedelic";
    static final String GENRE_GOTHIC = "哥特/Gothic";
    static final String GENRE_GARAGE = "车库/Garage";
    static final String GENRE_EXPERIMENTAL = "实验音乐/Experimental";
    static final String GENRE_NEW_WAVE = "新浪潮/New Wave";
    static final String GENRE_GRUNGE = "垃圾乐/Grunge";
    static final String GENRE_SADCORE = "悲核/Sadcore";
    static final String GENRE_EMO = "EMO/EMO";
    static final String GENRE_BLUES_ROCK = "Blues Rock/Blues - Rock";
    static final String GENRE_FUNK = "Funk/Funk";

    //民谣
    static final String GENRE_FOLK = "民谣/Folk";
    static final String GENRE_CAMPUS_FOLK = "校园民谣/Campus Folk";
    static final String GENRE_URBAN_FOLK = "城市民谣/Urban Folk";
    static final String GENRE_NEO_FOLK = "新民谣/Neo - Folk";
    static final String GENRE_COUNTRY = "乡村/Country";
    static final String GENRE_BLUEGRASS = "蓝草/Bluegrass";
    static final String GENRE_CELTIC = "凯尔特/Celtic";
    static final String GENRE_ANTI_FOLK = "Anti - folk/Anti - folk";

    //舞曲
    static final String GENRE_DANCE = "舞曲/Dance";
    static final String GENRE_BASS = "超重低音/Bass";
    static final String GENRE_DISCO = "迪士高/Disco";
    static final String GENRE_DJ = "DJ/DJ";
    static final String GENRE_E_DANCE = "电子舞曲/E-Dance";
    static final String GENRE_REMIX = "混音/Remix";
    static final String GENRE_BREAKBEAT = "碎拍/Breakbeat";
    static final String GENRE_HOUSE = "House/House";
    static final String GENRE_TRANCE = "Trance/Trance";
    static final String GENRE_PROGRESSIVE_HOUSE = "Progressive House/Progressive-House";
    static final String GENRE_PROGRESSIVE_TRANCE = "Progressive Trance/Progressive-Trance";
    static final String GENRE_DRUM_BASS = "Drum & Bass/Drum & Bass";
    static final String GENRE_CLUB_DANCE = "Club Dance/Club-Dance";

    //电子
    static final String GENRE_ELECTRONICA = "电子/Electronica";
    static final String GENRE_CHILLOUT = "驰放/Chillout";
    static final String GENRE_TRIP_HOP = "Trip-Hop/Trip-Hop";
    static final String GENRE_LOUNGE = "沙发/Lounge";
    static final String GENRE_DARKWAVE = "Darkwave/Darkwave";
    static final String GENRE_AMBIENT = "Ambient/Ambient";

    //古典音乐
    static final String GENRE_CLASSIC = "古典/Classic";
    static final String GENRE_CHAMBER_MUSIC = "室内乐/Chamber Music";
    static final String GENRE_SOLO = "独奏/Solo";
    static final String GENRE_OPERA = "歌剧/Opera";
    static final String GENRE_ART_MUSIC = "艺术歌曲/Art Music";
    static final String GENRE_SYMPHONY = "交响乐/Symphony";
    static final String GENRE_CONCERTO = "协奏曲/Concerto";
    static final String GENRE_SONATA = "奏鸣曲/Sonata";
    static final String GENRE_CONTEMPORARY_CHRISTIAN = "现代基督/Contemporary Christian";

    //爵士
    static final String GENRE_JAZZ = "爵士/Jazz";
    static final String GENRE_VOCAL_JAZZ = "人声爵士/Vocal Jazz";
    static final String GENRE_JAZZ_INSTRUMENTAL = "器乐爵士/Jazz instrumental";
    static final String GENRE_BIGBAND = "大乐队/BigBand";
    static final String GENRE_BOP = "波谱/Bop";
    static final String GENRE_HARD_BOP = "后波谱/HardBop";
    static final String GENRE_COOL_JAZZ = "冷爵士/Cool Jazz";
    static final String GENRE_FREE_JAZZ = "自由爵士/Free Jazz";
    static final String GENRE_FUSION_JAZZ = "融合爵士/Fusion Jazz";
    static final String GENRE_ACID_JAZZ = "酸爵士/Acid Jazz";
    static final String GENRE_BOSSA_NOVA = "Bossa Nova/Bossa - Nova";

    //其他
    static final String GENRE_VOCAL = "人声/Vocal";

    private static final String DEFAULT_PRESET = GENRE_NORMAL;

    static {
        PRESET_EQUALIZER_MAP.put(GENRE_NORMAL, new short[]{0, 0, 0, 0, 0, 0, 0, 0, 0, 0});

        /*流行*/
        PRESET_EQUALIZER_MAP.put(GENRE_POP, new short[]{0, 0, 400, 0, 0, -200, 0, 100, 300, 400});
        PRESET_EQUALIZER_MAP.put(GENRE_RB, new short[]{0, 0, 400, 200, -200, 0, 300, 0, 500, 200});
        PRESET_EQUALIZER_MAP.put(GENRE_HIP_HOP, new short[]{300, 300, 500, 200, 0, -200, 400, 0, 300, -200});
        PRESET_EQUALIZER_MAP.put(GENRE_RAP, new short[]{300, 100, 400, 300, 500, -500, 500, -300, 0, 600});
        PRESET_EQUALIZER_MAP.put(GENRE_BLUES, new short[]{-100, 600, 500, 200, 200, 0, 200, 300, 200, 300});
        PRESET_EQUALIZER_MAP.put(GENRE_CHANSON, new short[]{100, 0, 0, 100, 0, 300, 300, 200, 300, 200});
        PRESET_EQUALIZER_MAP.put(GENRE_SOUL, new short[]{-100, 600, 500, 400, -100, 400, 100, -100, 0, 0});
        PRESET_EQUALIZER_MAP.put(GENRE_REGGAE, new short[]{0, 400, 300, 100, 0, 0, 200, 200, 400, 0});
        PRESET_EQUALIZER_MAP.put(GENRE_PROGRESSIVE, new short[]{300, 200, 100, 200, 200, -200, -100, 200, 500, 300});
        PRESET_EQUALIZER_MAP.put(GENRE_GOSPEL, new short[]{0, 200, 200, 100, 100, 200, -100, 200, 300, 0});
        PRESET_EQUALIZER_MAP.put(GENRE_LATIN_POP, new short[]{100, 500, 500, 0, -100, 100, 0, 300, 400, 500});
        PRESET_EQUALIZER_MAP.put(GENRE_INDIE_POP, new short[]{400, -300, 0, 400, 100, 200, -500, 500, 600, 500});
        PRESET_EQUALIZER_MAP.put(GENRE_NEW_MEDIA, new short[]{0, 200, 300, 200, 100, 200, 100, 200, 100, 200});
        PRESET_EQUALIZER_MAP.put(GENRE_ALTERNATIVE_POP, new short[]{200, 300, 300, 100, 200, -200, -200, 300, 100, -100});
        PRESET_EQUALIZER_MAP.put(GENRE_TEEN_POP, new short[]{100, -100, -100, 200, 0, -100, 200, 300, 200, 300});
        PRESET_EQUALIZER_MAP.put(GENRE_DANCE_POP, new short[]{100, 400, 300, 200, 0, -100, -100, 200, 200, 500});
        PRESET_EQUALIZER_MAP.put(GENRE_J_POP, new short[]{0, 0, 0, 200, -100, 0, 100, 300, -200, 200});
        PRESET_EQUALIZER_MAP.put(GENRE_K_POP, new short[]{100, -100, 400, 400, -100, -100, 100, 400, 400, 100});
        PRESET_EQUALIZER_MAP.put(GENRE_DREAM_POP, new short[]{-100, -200, -200, -300, 0, 200, 400, -100, 200, -300});
        PRESET_EQUALIZER_MAP.put(GENRE_SYNTHPOP, new short[]{-300, -200, 0, -100, 400, 0, 300, 600, 400, 400});
        PRESET_EQUALIZER_MAP.put(GENRE_URBAN, new short[]{0, -100, 0, 100, 0, 100, 100, 100, 100, 100});

        /*摇滚*/
        PRESET_EQUALIZER_MAP.put(GENRE_ROCK, new short[]{-300, 0, 0, 300, 0, 200, 400, 200, 300, 0});
        PRESET_EQUALIZER_MAP.put(GENRE_POP_ROCK, new short[]{200, 300, 400, 300, 100, 100, 200, 400, 300, 200});
        PRESET_EQUALIZER_MAP.put(GENRE_BRIT_POP, new short[]{100, 300, 300, 300, 200, 100, -100, 400, 400, 100,});
        PRESET_EQUALIZER_MAP.put(GENRE_FOLK_ROCK, new short[]{0, 0, 100, 100, 100, 100, 100, 200, 0, 0});
        PRESET_EQUALIZER_MAP.put(GENRE_COUNTRY_ROCK, new short[]{0, -100, -100, 0, 100, 0, -100, -100, 0, 0});
        PRESET_EQUALIZER_MAP.put(GENRE_INDIE_ROCK, new short[]{0, 100, 200, 0, -100, -100, 100, 100, 100, -100,});
        PRESET_EQUALIZER_MAP.put(GENRE_ARTROCK, new short[]{-100, 200, 200, 200, 100, 100, 100, 100, 200, 200});
        PRESET_EQUALIZER_MAP.put(GENRE_HARD_ROCK, new short[]{0, 200, -100, 200, -100, 0, 300, 500, 200, 300});
        PRESET_EQUALIZER_MAP.put(GENRE_SOFT_ROCK, new short[]{-100, 100, 100, 0, 0, 200, 200, 100, 100, 0});
        PRESET_EQUALIZER_MAP.put(GENRE_ALTERNATIVE_ROCK, new short[]{100, 400, 400, 300, -100, 100, 200, 400, 400, 500});
        PRESET_EQUALIZER_MAP.put(GENRE_POSTROCK, new short[]{100, 300, -200, -100, -100, 0, 0, 100, 300, 0});
        PRESET_EQUALIZER_MAP.put(GENRE_HEAVY_METAL, new short[]{-100, 100, 200, 0, 0, 400, 300, 200, 300, 300});
        PRESET_EQUALIZER_MAP.put(GENRE_METAL, new short[]{-100, 200, 300, 200, 300, 100, 200, 200, 100, -100,});
        PRESET_EQUALIZER_MAP.put(GENRE_POP_METAL, new short[]{-100, 200, 300, 300, 300, 200, 200, 300, 300, -100});
        PRESET_EQUALIZER_MAP.put(GENRE_NU_METAL, new short[]{-200, 300, 100, -100, 0, 0, 300, 300, 200, 100});
        PRESET_EQUALIZER_MAP.put(GENRE_DEATH_METAL, new short[]{100, 500, 400, 500, -100, 300, 300, 400, 600, 600});
        PRESET_EQUALIZER_MAP.put(GENRE_NEO_CLASSICAL_METAL, new short[]{0, 100, 200, 300, -100, -100, 200, 300, 200, 0});
        PRESET_EQUALIZER_MAP.put(GENRE_PROGRESSIVE_METAL, new short[]{0, 100, 0, 200, 100, 0, 300, 200, 300, 200});
        PRESET_EQUALIZER_MAP.put(GENRE_INDUSTRIAL_METAL, new short[]{0, 0, 0, 100, 100, 0, 200, 300, 300, 300});
        PRESET_EQUALIZER_MAP.put(GENRE_BLACK_METAL, new short[]{-100, 300, 200, -100, 0, 0, 300, 300, 200, 100});
        PRESET_EQUALIZER_MAP.put(GENRE_GOTHIC_METAL, new short[]{0, -100, 300, 200, 100, 0, 200, 300, 0, 0});
        PRESET_EQUALIZER_MAP.put(GENRE_RAP_METAL, new short[]{100, 300, 300, 300, 100, 200, 300, 300, 200, -100});
        PRESET_EQUALIZER_MAP.put(GENRE_SYMPHONIC_METAL, new short[]{-100, 100, 200, 100, 200, 100, 200, 100, 100, -100});
        PRESET_EQUALIZER_MAP.put(GENRE_HARDCORE, new short[]{0, 200, 300, 300, 300, 100, 200, 300, 400, -200});
        PRESET_EQUALIZER_MAP.put(GENRE_POST_HARDCORE, new short[]{-100, 300, 400, 300, 300, 100, 200, 300, 400, -100});
        PRESET_EQUALIZER_MAP.put(GENRE_PUNK, new short[]{0, 200, 300, 200, 0, 400, 600, 400, 0, 500,});
        PRESET_EQUALIZER_MAP.put(GENRE_POST_PUNK, new short[]{100, 300, 200, 400, 200, 400, 0, -100, 200, -100});
        PRESET_EQUALIZER_MAP.put(GENRE_PUNK_POP, new short[]{-300, -200, 300, -100, -200, 300, 400, 500, 400, 400});
        PRESET_EQUALIZER_MAP.put(GENRE_POST_GRUNGE, new short[]{0, 200, 300, 200, -100, -100, 200, 200, 300, 0});
        PRESET_EQUALIZER_MAP.put(GENRE_PSYCHEDELIC, new short[]{100, 300, 300, 400, 200, 100, -100, 300, 200, 100});
        PRESET_EQUALIZER_MAP.put(GENRE_GOTHIC, new short[]{300, 300, 300, 300, 200, 100, 300, 400, 400, 300});
        PRESET_EQUALIZER_MAP.put(GENRE_GARAGE, new short[]{100, 200, 300, 200, 200, 100, 100, 200, 400, 400});
        PRESET_EQUALIZER_MAP.put(GENRE_EXPERIMENTAL, new short[]{-100, 0, 100, 0, 0, 0, 0, 100, 100, -100});
        PRESET_EQUALIZER_MAP.put(GENRE_NEW_WAVE, new short[]{100, 200, 200, 300, 200, 300, 0, -100, 100, -100});
        PRESET_EQUALIZER_MAP.put(GENRE_GRUNGE, new short[]{-100, 200, 300, 100, 0, 300, 300, 200, 300, 300});
        PRESET_EQUALIZER_MAP.put(GENRE_SADCORE, new short[]{0, 200, 100, 0, 0, 100, 200, 200, 100, 0});
        PRESET_EQUALIZER_MAP.put(GENRE_EMO, new short[]{0, 200, 200, 300, 200, 200, 200, 300, 400, 200});
        PRESET_EQUALIZER_MAP.put(GENRE_BLUES_ROCK, new short[]{-200, 100, 200, 200, 0, 200, 300, 200, 200, 0});
        PRESET_EQUALIZER_MAP.put(GENRE_FUNK, new short[]{400, 600, 600, 300, -100, 0, 0, 400, 300, 0});

        //民谣
        PRESET_EQUALIZER_MAP.put(GENRE_FOLK, new short[]{-100, -100, 100, -100, 200, -200, 100, 0, 400, 400});
        PRESET_EQUALIZER_MAP.put(GENRE_CAMPUS_FOLK, new short[]{-400, -200, -100, -100, -100, -200, 0, 200, -200, 100});
        PRESET_EQUALIZER_MAP.put(GENRE_URBAN_FOLK, new short[]{-200, -200, -100, -100, 0, 200, 100, 200, 200, -100});
        PRESET_EQUALIZER_MAP.put(GENRE_NEO_FOLK, new short[]{-200, -300, 100, -100, 200, 200, 100, 100, 300, 300});
        PRESET_EQUALIZER_MAP.put(GENRE_COUNTRY, new short[]{-200, -300, -100, -100, 0, 200, -200, 200, 200, -200,});
        PRESET_EQUALIZER_MAP.put(GENRE_BLUEGRASS, new short[]{-200, -100, 0, 100, 0, 200, 200, 200, 200, 100});
        PRESET_EQUALIZER_MAP.put(GENRE_CELTIC, new short[]{-100, 000, 100, -100, 200, 100, 300, 300, 300, 200});
        PRESET_EQUALIZER_MAP.put(GENRE_ANTI_FOLK, new short[]{-100, 200, 100, 100, 200, 100, 100, 0, 300, 200});

        //舞曲
        PRESET_EQUALIZER_MAP.put(GENRE_DANCE, new short[]{100, 400, 100, 400, 0, -200, 400, 0, 500, 500});
        PRESET_EQUALIZER_MAP.put(GENRE_BASS, new short[]{1300, 1100, -100, -100, 0, -400, 100, -300, 0, 0});
        PRESET_EQUALIZER_MAP.put(GENRE_DISCO, new short[]{0, 400, 500, 100, 0, 400, 600, 300, -200, 500});
        PRESET_EQUALIZER_MAP.put(GENRE_DJ, new short[]{-200, 0, 500, 600, 0, 200, 100, 300, 600, 400});
        PRESET_EQUALIZER_MAP.put(GENRE_E_DANCE, new short[]{100, 400, 200, 400, 0, -200, 400, 200, 500, 500});
        PRESET_EQUALIZER_MAP.put(GENRE_REMIX, new short[]{100, 200, 100, 300, 0, 200, 200, 000, 300, 200});
        PRESET_EQUALIZER_MAP.put(GENRE_BREAKBEAT, new short[]{-100, 200, 200, 300, 100, 100, 100, 300, 300, 300});
        PRESET_EQUALIZER_MAP.put(GENRE_HOUSE, new short[]{-100, 0, 300, 400, -100, 100, 200, 400, 500, 300});
        PRESET_EQUALIZER_MAP.put(GENRE_TRANCE, new short[]{300, 400, 400, 100, -100, 300, 500, 300, 400, 500});
        PRESET_EQUALIZER_MAP.put(GENRE_PROGRESSIVE_HOUSE, new short[]{-100, 0, 300, 300, 100, 300, 400, 400, 400, 300});
        PRESET_EQUALIZER_MAP.put(GENRE_PROGRESSIVE_TRANCE, new short[]{200, 300, 400, 100, -100, 300, 500, 400, 400, 500});
        PRESET_EQUALIZER_MAP.put(GENRE_DRUM_BASS, new short[]{200, 400, 400, 400, 0, 100, 300, 0, 200, 0});
        PRESET_EQUALIZER_MAP.put(GENRE_CLUB_DANCE, new short[]{100, 300, 300, 400, -200, -200, 300, 100, 500, 300});

        //电子
        PRESET_EQUALIZER_MAP.put(GENRE_ELECTRONICA, new short[]{-200, -100, -200, -200, -100, 100, 200, 400, 400, 300});
        PRESET_EQUALIZER_MAP.put(GENRE_CHILLOUT, new short[]{-100, -100, -200, -200, -100, 200, 200, 300, 200, 100});
        PRESET_EQUALIZER_MAP.put(GENRE_TRIP_HOP, new short[]{-200, -100, -100, 0, 0, 100, 100, 200, 100, 0});
        PRESET_EQUALIZER_MAP.put(GENRE_LOUNGE, new short[]{-200, -200, -100, -200, 100, 200, 200, 100, 100, 0});
        PRESET_EQUALIZER_MAP.put(GENRE_DARKWAVE, new short[]{100, 100, 100, 200, 200, 100, 200, 0, -100, -100});
        PRESET_EQUALIZER_MAP.put(GENRE_AMBIENT, new short[]{-200, 100, 200, 0, -100, 100, 200, 200, 400, 400});

        //古典音乐
        PRESET_EQUALIZER_MAP.put(GENRE_CLASSIC, new short[]{200, 100, 300, 300, -100, 100, 100, 400, 600, 300});
        PRESET_EQUALIZER_MAP.put(GENRE_CHAMBER_MUSIC, new short[]{200, 0, 300, -100, 200, 000, 100, 300, 400, 300});
        PRESET_EQUALIZER_MAP.put(GENRE_SOLO, new short[]{200, 0, 400, 0, 0, -100, 0, 500, 500, 0});
        PRESET_EQUALIZER_MAP.put(GENRE_OPERA, new short[]{300, -100, 200, 200, 100, 300, 300, 500, 400, 200});
        PRESET_EQUALIZER_MAP.put(GENRE_ART_MUSIC, new short[]{0, 0, 0, 0, 0, 300, 400, 400, 300, 100});
        PRESET_EQUALIZER_MAP.put(GENRE_SYMPHONY, new short[]{300, 100, 300, 200, 0, 100, 200, 500, 600, 300});
        PRESET_EQUALIZER_MAP.put(GENRE_CONCERTO, new short[]{200, 0, 200, 100, 100, 0, 200, 100, 100, 200});
        PRESET_EQUALIZER_MAP.put(GENRE_SONATA, new short[]{0, 0, 0, 0, 200, 300, 400, 400, 400, 200});
        PRESET_EQUALIZER_MAP.put(GENRE_CONTEMPORARY_CHRISTIAN, new short[]{400, 400, 200, 100, -200, -100, 200, 200, 400, 400});
        //爵士
        PRESET_EQUALIZER_MAP.put(GENRE_JAZZ, new short[]{200, 500, 400, 0, 0, 300, 300, -100, 400, 300});
        PRESET_EQUALIZER_MAP.put(GENRE_VOCAL_JAZZ, new short[]{100, 200, 200, 0, 0, 500, 500, -100, 400, 300});
        PRESET_EQUALIZER_MAP.put(GENRE_JAZZ_INSTRUMENTAL, new short[]{200, 600, 400, 0, 0, 400, 400, -100, 600, 300});
        PRESET_EQUALIZER_MAP.put(GENRE_BIGBAND, new short[]{200, 500, 300, 100, 100, 400, 400, 0, 300, 300});
        PRESET_EQUALIZER_MAP.put(GENRE_BOP, new short[]{300, 400, 400, 0, 0, 0, -100, -200, 200, 100});
        PRESET_EQUALIZER_MAP.put(GENRE_HARD_BOP, new short[]{200, 300, 300, 100, 0, 0, 200, 200, 100, 0});
        PRESET_EQUALIZER_MAP.put(GENRE_COOL_JAZZ, new short[]{300, 300, -100, 0, 0, 300, 300, -100, 300, 400});
        PRESET_EQUALIZER_MAP.put(GENRE_FREE_JAZZ, new short[]{200, 100, 100, 0, 0, 200, 200, 0, 200, 100});
        PRESET_EQUALIZER_MAP.put(GENRE_FUSION_JAZZ, new short[]{100, 0, 100, 0, 0, 100, 200, 200, 100, 0});
        PRESET_EQUALIZER_MAP.put(GENRE_ACID_JAZZ, new short[]{200, 400, 300, 100, 100, 300, 300, 0, 300, 200});
        PRESET_EQUALIZER_MAP.put(GENRE_BOSSA_NOVA, new short[]{100, 300, 400, 100, 0, 500, 500, 0, 300, 200});
        //其他
        PRESET_EQUALIZER_MAP.put(GENRE_VOCAL, new short[]{-100, 100, 100, 200, 200, 300, 300, 400, 300, 100});

        PRESET_NAME_LIST.addAll(PRESET_EQUALIZER_MAP.keySet());
    }

    /**
     * 获取流派列表
     *
     * @return 流派列表
     */
    public static List<String> getPresetNameList() {
        return PRESET_NAME_LIST;
    }

    /**
     * 获取style 名称
     * @param styleId styleId
     * @return style名称
     */
    public static String getStyleName(int styleId) {
        if (styleId >= CUSTOME_STYLE) {
            return GENRE_CUSTOM;
        } else {
            return PRESET_NAME_LIST.get(styleId < 0 ? 0 : styleId);
        }
    }

    /**
     * 获取style id
     * @param styleName style名称
     * @return style id
     */
    public static int getStyleId(String styleName) {
        int styleId = EqualizerPreset.getPresetNameList().indexOf(styleName);
        if (styleId == -1) {
            styleId = EqualizerPreset.CUSTOME_STYLE;
        }
        return styleId;
    }

    /**
     * 获取EQ数据
     *
     * @param presetName presetName
     * @return EQ数据
     */
    public static short[] getEqualizerData(String presetName) {
        return PRESET_EQUALIZER_MAP.get(presetName);
    }

    /**
     * 获取默认EQ的key值
     *
     * @return 默认EQ key
     */
    public static String getDefaultPresetName() {
        return GENRE_NORMAL;
    }
}

