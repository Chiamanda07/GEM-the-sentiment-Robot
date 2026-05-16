# emotion_server.py
# Run this on your PC or Raspberry Pi on the same WiFi network as the ESP32.
# Install deps: pip install flask scikit-learn
#
# Then run: python emotion_server.py
# It will print the IP address to put into your ESP32 firmware.

from flask import Flask, request, jsonify
import pickle
import socket
import torch
from transformers import DistilBertTokenizerFast, DistilBertForSequenceClassification

app = Flask(__name__)

# ---------- Load DistilBERT model bundle ----------
MODEL_PKL = "emotion_model.pkl"

with open(MODEL_PKL, "rb") as f:
    bundle = pickle.load(f)

le        = bundle["label_encoder"]
tokenizer = DistilBertTokenizerFast.from_pretrained(bundle["model_dir"])
model     = DistilBertForSequenceClassification.from_pretrained(bundle["model_dir"])
model.eval()

def predict(text):
    inputs = tokenizer(text, return_tensors="pt",
                       truncation=True, padding=True, max_length=128)
    with torch.no_grad():
        logits = model(**inputs).logits
    pred_id = torch.argmax(logits, dim=1).item()
    return le.inverse_transform([pred_id])[0]

# ---------- Emotion endpoint ----------
@app.route("/emotion", methods=["POST"])
def predict_emotion():
    data = request.get_json()
    if not data or "text" not in data:
        return jsonify({"error": "No text provided"}), 400

    text = data["text"]

    # If your model is a full sklearn Pipeline (includes vectorizer):
    prediction = model.predict([text])[0]

    # If your model needs a separate vectorizer, use this instead:
    # features = vectorizer.transform([text])
    # prediction = model.predict(features)[0]

    # Normalize output to one of the 5 emotions the ESP32 expects
    emotion_map = {
        "happy":     "HAPPY",
        "joy":       "HAPPY",
        "sad":       "SAD",
        "sadness":   "SAD",
        "angry":     "ANGRY",
        "anger":     "ANGRY",
        "fear":      "FEAR",
        "scared":    "FEAR",
        "thinking":  "THINKING",
        "curious":   "THINKING",
        "confused":  "CONFUSED",
        "excited":   "EXCITED",
        "surprise":  "EXCITED",
        "surprised": "EXCITED",
        "neutral":   "NEUTRAL",
    }

    normalized = emotion_map.get(str(prediction).lower(), "NEUTRAL")
    return jsonify({"emotion": normalized, "raw": str(prediction)})

# ---------- Run ----------
if __name__ == "__main__":
    # Print local IP so you know what to put in the ESP32
    hostname = socket.gethostname()
    local_ip = socket.gethostbyname(hostname)
    print(f"\n✅ Emotion API running!")
    print(f"   Put this in your ESP32 firmware:")
    print(f"   const char* EMOTION_API = \"http://{local_ip}:5000/emotion\";\n")
    app.run(host="0.0.0.0", port=5000)
