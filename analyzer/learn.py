import tensorflow as tf
from tensorflow.keras.preprocessing.image import ImageDataGenerator
from tensorflow.keras.applications import MobileNetV2
from tensorflow.keras.layers import Dense, GlobalAveragePooling2D
from tensorflow.keras.models import Model
from tensorflow.keras.optimizers import Adam

# Verwende ImageDataGenerator, um die Bilder aus deinen Ordnern zu laden und für das Training vorzubereiten.
train_datagen = ImageDataGenerator(
    rescale=1.0/255,
    shear_range=0.2,
    zoom_range=0.2,
    horizontal_flip=True,
    validation_split=0.2)  # 20% der Daten für die Validierung

train_generator = train_datagen.flow_from_directory(
    'train_data/',
    target_size=(224, 224),  # Größe der Bilder, die in das Modell eingegeben werden
    batch_size=32,
    class_mode='binary',  # Da wir nur zwei Klassen haben
    subset='training')

validation_generator = train_datagen.flow_from_directory(
    'train_data/',
    target_size=(224, 224),
    batch_size=32,
    class_mode='binary',
    subset='validation')

# Wir verwenden MobileNetV2, ein leichtes und effizientes Modell, das gut für Transfer Learning geeignet ist.
base_model = MobileNetV2(weights='imagenet', include_top=False, input_shape=(224, 224, 3))

# Jetzt erweitern wir das Modell um unsere eigenen Schichten:
x = base_model.output
x = GlobalAveragePooling2D()(x)
x = Dense(128, activation='relu')(x)  # Eine versteckte Schicht
predictions = Dense(1, activation='sigmoid')(x)  # Ausgabe für binäre Klassifikation

model = Model(inputs=base_model.input, outputs=predictions)

# Wir kompilieren das Modell, bevor wir es trainieren:
# Wir "frieren" die Schichten des vortrainierten Modells ein, damit sie nicht aktualisiert werden
for layer in base_model.layers:
    layer.trainable = False

model.compile(optimizer=Adam(learning_rate=0.001), loss='binary_crossentropy', metrics=['accuracy'])

# Nun trainieren wir das Modell mit den neuen Daten:
model.fit(
    train_generator,
    steps_per_epoch=train_generator.samples // train_generator.batch_size,
    validation_data=validation_generator,
    validation_steps=validation_generator.samples // validation_generator.batch_size,
    epochs=10)

# Nach dem initialen Training kannst du einige oder alle Schichten des vortrainierten Modells "auftauen" und die Lernrate reduzieren, um das Modell weiter zu verbessern:
for layer in base_model.layers:
    layer.trainable = True

model.compile(optimizer=Adam(learning_rate=0.0001), loss='binary_crossentropy', metrics=['accuracy'])

model.fit(
    train_generator,
    steps_per_epoch=train_generator.samples // train_generator.batch_size,
    validation_data=validation_generator,
    validation_steps=validation_generator.samples // validation_generator.batch_size,
    epochs=10)

# Speichere das trainierte Modell zur späteren Verwendung:
model.save('briefkasten_model.h5')
