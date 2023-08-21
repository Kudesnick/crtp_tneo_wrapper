#pragma once

#include <stdint.h>
#include "os.h"
#include "misc.h"
#include "csp_spi.h"

namespace bsp
{

class mx25
{
protected:
    os::mutex mutex;
    static inline os::eventgrp waitevnt;
    const uint32_t event_pattern;
    static inline constexpr auto def_delay = 500;

    friend void csp::spi::cb_complete(void);

    enum class cmd: uint8_t
    {
        // Read/Write Array Commands
        READ            = 0x03, // normal read
        FAST_READ       = 0x0B, // fast read data
        PP              = 0x02, // page program
        SE              = 0x20, // sector erase
        BE_32K          = 0x52, // block erase 32KB
        BE              = 0xD8, // block erase 64KB
        CE              = 0xC7, // chip erase (0x60 or 0xC7)
        // Register/Setting Commands
        WREN            = 0x06, // write enable
        WRDI            = 0x04, // write disable
        fFMEN           = 0x41, // factory mode enable
        RDSR            = 0x05, // read status register
        RDCR            = 0x15, // read configuration register
        WRSR            = 0x01, // write status/configuration register
        WPSEL           = 0x68, // Write Protect Selection
        PGM_ERS_Suspend = 0xB0, // Suspends Program/Erase
        PGM_ERS_Resume  = 0x30, // Resumes Program/Erase
        DP              = 0xB9, // Deep power down
        RDP             = 0xAB, // Release from deep power down
        SBL             = 0xC0, // Set Burst Length
        // ID/Security Commands
        RDID            = 0x9F, // read identification
        RES             = 0xAB, // read electronic ID
        REMS            = 0x90, // read electronic manufacturer & device ID
        RDSFDP          = 0x5A, // Read SFDP Mode (JEDEC Standard)
        ENSO            = 0xB1, // enter secured OTP
        EXSO            = 0xC1, // exit secured OTP
        RDSCUR          = 0x2B, // read security register
        WRSCUR          = 0x2F, // write security register
        WRSPB           = 0xE3, // SPB bit program
        ESSPB           = 0xE4, // all SPB bit erase
        RDSPB           = 0xE2, // read SPB status
        WRDPB           = 0xE1, // write DPB register
        RDDPB           = 0xE0, // read DPB registe
        GBLK            = 0x7E, // gang block lock
        GBULK           = 0x98, // gang block unlock
        WRLR            = 0x2C, // write lock register
        RDLR            = 0x2D, // read lock register
        // Reset Commands
        NOP             = 0x00, // No Operation
        RSTEN           = 0x66, // Reset Enable
        RST             = 0x99, // Reset Memory
    };
    
    void send_wait(const uint32_t _tout = def_delay);
    void send_cmd(const cmd _cmd);
public:
    res reset(const cmd _reset_for = cmd::NOP);

    struct id
    {
        uint8_t manufacturer_id;
        uint8_t type;
        uint8_t density;
    };
    res read_id(id &_id);

    mx25(const uint32_t _evnt_pattern = 1);
    ~mx25();
};

}; // namespace bsp
