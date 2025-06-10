#pragma once

#include <Arduino.h>
#include "FtpClientWrapper.h"

class FirmwareUpdater {
public:
    /**
     * Konstruktor übernimmt Wrapper und Update-Dateiname
     * @param ftpWrapper Referenz auf FtpClientWrapper
     * @param updateFile Pfad der Firmware-Datei auf FTP
     */
    FirmwareUpdater(FtpClientWrapper& ftpWrapper, const char* updateFile);

    /**
     * Prüft, ob auf dem Server eine neue Firmware vorliegt und spielt sie ein.
     */
    void checkAndUpdate();

private:
    FtpClientWrapper& _ftp;
    const char* _updateFile;
};
