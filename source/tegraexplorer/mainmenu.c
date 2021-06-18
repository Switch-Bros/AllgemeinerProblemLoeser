#include "mainmenu.h"
#include "../gfx/gfx.h"
#include "../gfx/gfxutils.h"
#include "../gfx/menu.h"
#include "tools.h"
#include "../hid/hid.h"
#include "../fs/menus/explorer.h"
#include <utils/btn.h>
#include <storage/nx_sd.h>
#include "tconf.h"
#include "../keys/keys.h"
#include "../storage/mountmanager.h"
#include "../storage/gptmenu.h"
#include "../storage/emummc.h"
#include <utils/util.h>
#include "../fs/fsutils.h"
#include <soc/fuse.h>
#include "../utils/utils.h"
#include "../config.h"

#include "../fs/readers/folderReader.h"
#include "../fs/fstypes.h"
#include "../fs/fscopy.h"

extern hekate_config h_cfg;

enum {
    MainExplore = 0,
    DeleteBootFlags,
    DeleteThemes,
    FixClingWrap,
    FixAll,
    MainOther,
    // MainBrowseSd,
    // MainMountSd,
    // MainBrowseEmmc,
    // MainBrowseEmummc,
    // MainTools,
    // MainPartitionSd,
    // MainDumpFw,
    // MainViewKeys,
    MainViewCredits,
    MainExit,
    MainPowerOff,
    MainRebootRCM,
    // MainRebootNormal,
    MainRebootHekate,
    // MainRebootAMS,
};

MenuEntry_t mainMenuEntries[] = {
    [MainExplore] = {.optionUnion = COLORTORGB(COLOR_WHITE) | SKIPBIT, .name = "-- Tools --"},
    [DeleteBootFlags] = {.optionUnion = COLORTORGB(COLOR_GREEN), .name = "Delete boot2.flags"},
    [DeleteThemes] = {.optionUnion = COLORTORGB(COLOR_GREEN), .name = "Delete installed themes"},
    [FixClingWrap] = {.optionUnion = COLORTORGB(COLOR_GREEN), .name = "Fix ClingWrap"},
    [FixAll] = {.optionUnion = COLORTORGB(COLOR_GREEN), .name = "Try everything"},
    
    // [MainBrowseSd] = {.optionUnion = COLORTORGB(COLOR_GREEN), .name = "Browse SD"},
    // [MainMountSd] = {.optionUnion = COLORTORGB(COLOR_YELLOW)}, // To mount/unmount the SD
    // [MainBrowseEmmc] = {.optionUnion = COLORTORGB(COLOR_BLUE), .name = "Browse EMMC"},
    // [MainBrowseEmummc] = {.optionUnion = COLORTORGB(COLOR_BLUE), .name = "Browse EMUMMC"},
    // [MainTools] = {.optionUnion = COLORTORGB(COLOR_WHITE) | SKIPBIT, .name = "\n-- Tools --"},
    // [MainPartitionSd] = {.optionUnion = COLORTORGB(COLOR_ORANGE), .name = "Partition the sd"},
    // [MainDumpFw] = {.optionUnion = COLORTORGB(COLOR_BLUE), .name = "Dump Firmware"},
    // [MainViewKeys] = {.optionUnion = COLORTORGB(COLOR_YELLOW), .name = "View dumped keys"},
    [MainOther] = {.optionUnion = COLORTORGB(COLOR_WHITE) | SKIPBIT, .name = "\n-- Other --"},
    [MainViewCredits] = {.optionUnion = COLORTORGB(COLOR_YELLOW), .name = "Credits"},
    [MainExit] = {.optionUnion = COLORTORGB(COLOR_WHITE) | SKIPBIT, .name = "\n-- Exit --"},
    [MainPowerOff] = {.optionUnion = COLORTORGB(COLOR_VIOLET), .name = "Power off"},
    [MainRebootRCM] = {.optionUnion = COLORTORGB(COLOR_VIOLET), .name = "Reboot to RCM"},
    // [MainRebootNormal] = {.optionUnion = COLORTORGB(COLOR_VIOLET), .name = "Reboot normally"},
    [MainRebootHekate] = {.optionUnion = COLORTORGB(COLOR_VIOLET), .name = "Reboot to bootloader/update.bin"},
    // [MainRebootAMS] = {.optionUnion = COLORTORGB(COLOR_VIOLET), .name = "Reboot to atmosphere/reboot_payload.bin"}
};

void HandleSD(){
    gfx_clearscreen();
    TConf.curExplorerLoc = LOC_SD;
    if (!sd_mount() || sd_get_card_removed()){
        gfx_printf("Sd is not mounted!");
        hidWait();
    }
    else
        FileExplorer("sd:/");
}

void HandleEMMC(){
   GptMenu(MMC_CONN_EMMC);
}

void HandleEMUMMC(){
    GptMenu(MMC_CONN_EMUMMC);
}

void ViewKeys(){
    gfx_clearscreen();
    for (int i = 0; i < 3; i++){
        gfx_printf("\nBis key 0%d:   ", i);
        PrintKey(dumpedKeys.bis_key[i], AES_128_KEY_SIZE * 2);
    }
    
    gfx_printf("\nMaster key 0: ");
    PrintKey(dumpedKeys.master_key, AES_128_KEY_SIZE);
    gfx_printf("\nHeader key:   ");
    PrintKey(dumpedKeys.header_key, AES_128_KEY_SIZE * 2);
    gfx_printf("\nSave mac key: ");
    PrintKey(dumpedKeys.save_mac_key, AES_128_KEY_SIZE);

    u8 fuseCount = 0;
    for (u32 i = 0; i < 32; i++){
        if ((fuse_read_odm(7) >> i) & 1)
            fuseCount++;
    }

    gfx_printf("\n\nPkg1 ID: '%s' (kb %d)\nFuse count: %d", TConf.pkg1ID, TConf.pkg1ver, fuseCount);

    hidWait();
}

void ViewCredits(){
    gfx_clearscreen();
    gfx_printf("\nCommon Problem Resolver v%d.%d.%d\nBy Team Neptune\n\nBased on TegraExplorer by SuchMemeManySkill,\nLockpick_RCM & Hekate, from shchmue & CTCaer\n\n\n", LP_VER_MJ, LP_VER_MN, LP_VER_BF);
    hidWait();
}

extern bool sd_mounted;
extern bool is_sd_inited;
extern int launch_payload(char *path);

void RebootToAMS(){
    launch_payload("sd:/atmosphere/reboot_payload.bin");
}

void RebootToHekate(){
    launch_payload("sd:/bootloader/update.bin");
}

void MountOrUnmountSD(){
    gfx_clearscreen();
    if (sd_mounted)
        sd_unmount();
    else if (!sd_mount())
        hidWait();
}


void DeleteFileSimple(char *thing){
    //char *thing = CombinePaths(path, entry.name);
    int res = f_unlink(thing);
    if (res)
        DrawError(newErrCode(res));
    free(thing);
}

void deleteBootFlags(){
    gfx_clearscreen();
    char *storedPath = CpyStr("sd:/atmosphere/contents");
    int readRes = 0;
    Vector_t fileVec = ReadFolder(storedPath, &readRes);
    if (readRes){
        clearFileVector(&fileVec);
        DrawError(newErrCode(readRes));
    } else {
        vecDefArray(FSEntry_t*, fsEntries, fileVec);
        for (int i = 0; i < fileVec.count; i++){

            char *suf = "/flags/boot2.flag";
            char *flagPath = CombinePaths(storedPath, fsEntries[i].name);
            flagPath = CombinePaths(flagPath, suf);

            if (FileExists(flagPath)) {
                gfx_printf("Deleting: %s\n", flagPath);
                DeleteFileSimple(flagPath);
            }
            free(flagPath);
        }
    }
    gfx_printf("\n\n Done, press a key to proceed.");
    hidWait();
}

void deleteTheme(char* basePath, char* folderId){
    char *path = CombinePaths(basePath, folderId);
    if (FileExists(path)) {
        gfx_printf("-- Theme found: %s\n", path);
        FolderDelete(path);
    }
    free(path);
}

void deleteInstalledThemes(){
    gfx_clearscreen();
    deleteTheme("sd:/atmosphere/contents", "0100000000001000");
    deleteTheme("sd:/atmosphere/contents", "0100000000001007");
    deleteTheme("sd:/atmosphere/contents", "0100000000001013");

    gfx_printf("\n\n Done, press a key to proceed.");
    hidWait();
}

void fixClingWrap(){
    gfx_clearscreen();
    char *bpath = CpyStr("sd:/_b0otloader");
    char *bopath = CpyStr("sd:/bootloader");
    char *kpath = CpyStr("sd:/atmosphere/_k1ps");
    char *kopath = CpyStr("sd:/atmosphere/kips");

    if (FileExists(bpath)) {
        if (FileExists(bopath)) {
            FolderDelete(bopath);
        }
        int res = f_rename(bpath, bopath);
        if (res){
            DrawError(newErrCode(res));
        }
        gfx_printf("-- Fixed Bootloader\n");
    }

    if (FileExists(kpath)) {
        if (FileExists(kopath)) {
            FolderDelete(kopath);
        }
        int res = f_rename(bpath, kopath);
        if (res){
            DrawError(newErrCode(res));
        }
        gfx_printf("-- Fixed kips\n");
    }

    gfx_printf("\n\n Done, press a key to proceed.");
    hidWait();
}

void fixAll(){
    gfx_clearscreen();
    deleteBootFlags();
    deleteInstalledThemes();
    fixClingWrap();
}

menuPaths mainMenuPaths[] = {
    [DeleteBootFlags] = deleteBootFlags,
    [DeleteThemes] = deleteInstalledThemes,
    [FixClingWrap] = fixClingWrap,
    [FixAll] = fixAll,
    // [MainBrowseSd] = HandleSD,
    // [MainMountSd] = MountOrUnmountSD,
    // [MainBrowseEmmc] = HandleEMMC,
    // [MainBrowseEmummc] = HandleEMUMMC,
    // [MainPartitionSd] = FormatSD,
    // [MainDumpFw] = DumpSysFw,
    // [MainViewKeys] = ViewKeys,
    // [MainRebootAMS] = RebootToAMS,
    [MainRebootHekate] = RebootToHekate,
    [MainRebootRCM] = reboot_rcm,
    [MainPowerOff] = power_off,
    [MainViewCredits] = ViewCredits,
    // [MainRebootNormal] = reboot_normal
};

void EnterMainMenu(){
    int res = 0;
    while (1){
        if (sd_get_card_removed())
            sd_unmount();

        // // -- Explore --
        // mainMenuEntries[MainBrowseSd].hide = !sd_mounted;
        // mainMenuEntries[MainMountSd].name = (sd_mounted) ? "Unmount SD" : "Mount SD";
        // mainMenuEntries[MainBrowseEmummc].hide = (!emu_cfg.enabled || !sd_mounted);

        // // -- Tools --
        // mainMenuEntries[MainPartitionSd].hide = (!is_sd_inited || sd_get_card_removed());
        // mainMenuEntries[MainDumpFw].hide = (!TConf.keysDumped || !sd_mounted);
        // mainMenuEntries[MainViewKeys].hide = !TConf.keysDumped;

        // // -- Exit --
        // mainMenuEntries[MainRebootAMS].hide = (!sd_mounted || !FileExists("sd:/atmosphere/reboot_payload.bin"));
        mainMenuEntries[MainRebootHekate].hide = (!sd_mounted || !FileExists("sd:/bootloader/update.bin"));
        mainMenuEntries[MainRebootRCM].hide = h_cfg.t210b01;

        gfx_clearscreen();
        gfx_putc('\n');
        
        Vector_t ent = vecFromArray(mainMenuEntries, ARR_LEN(mainMenuEntries), sizeof(MenuEntry_t));
        res = newMenu(&ent, res, 79, 30, ALWAYSREDRAW, 0);
        if (mainMenuPaths[res] != NULL)
            mainMenuPaths[res]();
    }
}
