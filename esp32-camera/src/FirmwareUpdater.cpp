#include "FirmwareUpdater.h"
#include <Update.h>

// FirmwareUpdater::FirmwareUpdater(FtpClientWrapper& ftpWrapper, const char* updateFile)
//     : _ftp(ftpWrapper), _updateFile(updateFile) {}

// void FirmwareUpdater::checkAndUpdate() {
//     if (!_ftp.connect()) return;
//     _ftp.changeDir("/");
//     size_t size = _ftp.fileSize(_updateFile);
//     if (size == 0) {
//         _ftp.disconnect();
//         return;
//     }
//     _ftp.download(_updateFile, [&](Stream& stream, size_t) {
//         if (Update.begin(size)) {
//             Update.onProgress([](size_t written, size_t total) {
//                 Serial.printf("%u%%\n", written * 100 / total);
//             });
//             size_t written = Update.writeStream(stream);
//             if (written == size && Update.end()) {
//                 Serial.println("Firmware erfolgreich aktualisiert");
//                 String backup = String(_updateFile) + ".bak";
//                 _ftp.renameFile(_updateFile, backup.c_str());
//                 ESP.restart();
//             } else {
//               Serial.println("Firmware-Update fehlgeschlagen");
//             }
//         } else {
//             Serial.println("Update.begin() fehlgeschlagen");
//         }
//     });
//     _ftp.disconnect();
// }