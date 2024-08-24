from tensorflow.keras.models import load_model

model = load_model('briefkasten_model.h5')

# Beispielhafte Vorhersage
import numpy as np
from tensorflow.keras.preprocessing import image

img = image.load_img('path_to_image.jpg', target_size=(224, 224))
img_array = image.img_to_array(img)
img_array = np.expand_dims(img_array, axis=0)
img_array /= 255.0  # Normalisierung wie beim Training

prediction = model.predict(img_array)
print('Leer' if prediction[0] < 0.5 else 'Gefüllt')
