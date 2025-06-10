#include "FtpClientWrapper.h"

FtpClientWrapper::FtpClientWrapper(const char* server, const char* user, const char* pass)
    : _ftp(server, user, pass), _connected(false) {}

bool FtpClientWrapper::connect() {
  if (_connected) return true;
  _ftp.enableDebug();
  _ftp.OpenConnection();
  _connected = true;
  return true;
}

void FtpClientWrapper::disconnect() {
  if (_connected) {
    _ftp.CloseConnection();
    _connected = false;
  }
}

bool FtpClientWrapper::changeDir(const char* dir) {
  if (!_connected && !connect()) return false;
  _ftp.ChangeWorkDir(dir);
  return true;
}

bool FtpClientWrapper::upload(const uint8_t* buf, size_t len, const char* filename) {
  if (!_connected && !connect()) return false;
  _ftp.InitFile("Type I");
  _ftp.NewFile(filename);
  _ftp.WriteData((unsigned char*)buf, len);
  return true;
}

size_t FtpClientWrapper::fileSize(const char* filename) {
  if (!_connected && !connect()) return 0;
  return _ftp.getSize(filename);
}

bool FtpClientWrapper::download(const char* filename, DownloadCallback callback) {
  if (!_connected && !connect()) return false;
  _ftp.InitFile("Type I");
  Stream* stream = _ftp.requestFile(filename);
  if (!stream) return false;
  size_t size = fileSize(filename);
  callback(*stream, size);
  return true;
}

bool FtpClientWrapper::renameFile(const char* oldName, const char* newName) {
  if (!_connected && !connect()) return false;
  _ftp.RenameFile(oldName, newName);

  return true;
}