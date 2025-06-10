// server.js
const express = require('express');
const multer  = require('multer');
const path    = require('path');
const fs      = require('fs');

// === Ordner definieren ===
const IMG_DIR      = path.join(__dirname, 'upload');

// Bei Bedarf Ordner anlegen
for (const dir of [IMG_DIR]) {
  if (!fs.existsSync(dir)) {
    fs.mkdirSync(dir, { recursive: true });
    console.log(`Ordner erstellt: ${dir}`);
  }
}

// === Multer-Storage für Bild-Upload mit vorgegebenem Dateinamen ===
const storage = multer.diskStorage({
  destination: (req, file, cb) => {
    cb(null, IMG_DIR);
  },
  filename: (req, file, cb) => {
    // gewählten Namen holen und absichern
    let name = req.body.filename;
    if (!name) {
      return cb(new Error('Kein Dateiname im Feld "filename" angegeben'));
    }
    name = path.basename(name); // schützt gegen Pfad-Manipulation

    // Dateiendung prüfen / übernehmen
    const origExt = path.extname(file.originalname);
    const hasExt  = path.extname(name) !== '';
    const finalName = hasExt ? name : name + origExt;

    cb(null, finalName);
  }
});
const upload = multer({ storage });

// === Express-App ===
const app = express();

// POST /img
// Form-Data:
//   • Feld "image"  = Datei
//   • Feld "filename" = gewünschter Name (z.B. "meinBild.jpg")
app.post('/upload', upload.single('image'), (req, res) => {
  if (!req.file) {
    return res.status(400).json({ error: 'Keine Datei hochgeladen.' });
  }
  res.json({
    message: 'Upload erfolgreich',
    file: {
      filename: req.file.filename,
      size: req.file.size
    }
  });
});


// Server starten
const PORT = process.env.PORT || 3000;
app.listen(PORT, () => {
  console.log(`Server läuft auf http://localhost:${PORT}`);
});
