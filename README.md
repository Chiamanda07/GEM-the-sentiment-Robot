# GEM-the-sentiment-Robot
This robot embodies an AI system by integrating the Gemini API with an ESP32 allowing someone to talk to it and get a response back. The robot's facial expressions are connected to a sentiment analysis model to interpret different emotions. I'm using this project to explore how natural language processing can help robots better understand human emotion and communication.

# Electronic Aspect
For the electrical components I'm using a 
- TFT Display
- ESP32-S3 Development Board
- INMP441 I2S Digital Microphone
- MAX98357A I2S Audio Amplifier
- 8 Ohm 1W Small Speaker
- 18650 battery

I couldn't really find a simulation software that had all the parts to show the connection, but I've explained it in the table below:
| Component               | Pin  | ESP32-S3 Pin | Purpose                              |
| ----------------------- | ---- | ------------ | ------------------------------------ |
| **Common**              | GND  | GND          | Shared ground for all components     |
|                         | 3.3V | 3.3V         | Power for TFT display and microphone |
|                         | 5V   | 5V           | Power for amplifier                  |
| **TFT Display (SPI)**   | VCC  | 3.3V         | Screen power                         |
|                         | GND  | GND          | Ground                               |
|                         | SCK  | GPIO36       | SPI clock                            |
|                         | MOSI | GPIO35       | SPI data from ESP32                  |
|                         | CS   | GPIO34       | Chip select                          |
|                         | DC   | GPIO33       | Data / command control               |
|                         | RST  | GPIO37       | Reset pin                            |
|                         | BL   | 3.3V         | Backlight power                      |
| **INMP441 Microphone**  | SD   | GPIO14       | I2S data input                       |
|                         | WS   | GPIO16       | I2S word select                      |
|                         | SCK  | GPIO15       | I2S bit clock                        |
|                         | L/R  | GND          | Sets microphone to left channel      |
| **MAX98357A Amplifier** | DIN  | GPIO17       | I2S audio data output                |
|                         | LRC  | GPIO16       | Shared with microphone WS            |
|                         | BCLK | GPIO15       | Shared with microphone SCK           |
|                         | VIN  | 5V           | Amplifier power                      |
|                         | GND  | GND          | Ground                               |


# The Body of GEM
For the body, I used fusion 360 to give it a cloud shaped body. The holes on the model are for the face(TFT Display screen), microphone, and speaker.
![Cloud body model](Model_picture.png)
This is the link to getting the full .f3d file for the body:
[Download the Fusion 360 body file](GEM_Model_f3d.f3d)

# Initial Arduino code
[This code](GEM_initial_code.ino) is an initial code for the the different parts.
In this code some of the things have not been filled for privacy reasons such as 
- my wifi id
- my wifi password
- my gemini api key and url.

The Emotion_API is simply the address the ESP32 uses to reach my Python emotion server running on my PC.
I'm currently still working on the emotion model, but it would be something similar to my [book review model](https://github.com/Chiamanda07/ML_bookReview_Project) I made some time ago, but with much more labels (emotions).

# BOM
## Bill of Materials (BOM)

| Component | Price (USD) | Purchase Link |
|---|---|---|
| TFT Display Screen | 4.22 | [Aliexpress](https://www.aliexpress.us/item/3256808661885479.html?spm=a2g0o.productlist.main.1.67a635v335v3iI&algo_pvid=bcaee0e2-25e7-4fe4-bec4-695d47b157ad&algo_exp_id=bcaee0e2-25e7-4fe4-bec4-695d47b157ad-0&pdp_ext_f=%7B%22order%22%3A%22418%22%2C%22eval%22%3A%221%22%2C%22fromPage%22%3A%22search%22%7D&pdp_npi=6%40dis%21USD%214.23%210.99%21%21%2129.11%216.80%21%402101ee6617726414750976499eadfc%2112000046932083108%21sea%21US%210%21ABX%211%210%21n_tag%3A-29910%3Bd%3Ae37b4f27%3Bm03_new_user%3A-29895%3BpisId%3A5000000197847475&curPageLogUid=mbVpQz6Wbfg1&utparam-url=scene%3Asearch%7Cquery_from%3A#nav-specification) |
| ESP32-S3 Development Board | 7.59 | [Aliexpress](https://www.aliexpress.us/item/3256807465857681.html?aff_fcid=eb81d8d9df1e4328a2b95461bc347583-1773105321230-01718-_c3qOT5g3&tt=CPS_NORMAL&aff_fsk=_c3qOT5g3&aff_platform=shareComponent-detail&sk=_c3qOT5g3&aff_trace_key=eb81d8d9df1e4328a2b95461bc347583-1773105321230-01718-_c3qOT5g3&terminal_id=dc91d0876e694c4eba7c1a56dc7739ed&afSmartRedirect=y&gatewayAdapt=glo2usa4itemAdapt) |
| INMP441 I2S Digital Microphone | 5.53 | [Aliexpress](https://www.aliexpress.us/item/3256807120331768.html?aff_fcid=38914325d00a471bbfa3345abc315c40-1773105605528-02184-_c4tN51Kb&tt=CPS_NORMAL&aff_fsk=_c4tN51Kb&aff_platform=shareComponent-detail&sk=_c4tN51Kb&aff_trace_key=38914325d00a471bbfa3345abc315c40-1773105605528-02184-_c4tN51Kb&terminal_id=dc91d0876e694c4eba7c1a56dc7739ed&afSmartRedirect=y&gatewayAdapt=glo2usa4itemAdapt) |
| MAX98357A I2S Audio Amplifier | 7.63 | [Aliexpress](https://www.aliexpress.us/item/3256809415571726.html?aff_fcid=39fb7258209f4fa690389f7b6a186d9f-1773105448760-08631-_c4eLWXJ9&tt=CPS_NORMAL&aff_fsk=_c4eLWXJ9&aff_platform=shareComponent-detail&sk=_c4eLWXJ9&aff_trace_key=39fb7258209f4fa690389f7b6a186d9f-1773105448760-08631-_c4eLWXJ9&terminal_id=dc91d0876e694c4eba7c1a56dc7739ed&afSmartRedirect=y&gatewayAdapt=glo2usa4itemAdapt) |
| 8 Ohm 1W Small Speaker | 5.19 | [Aliexpress](https://www.aliexpress.us/item/3256811425287668.html?spm=a2g0o.productlist.main.14.111aV0wvV0wvL2&algo_pvid=8577ada1-8396-452a-a38c-017ff3a15e99&algo_exp_id=8577ada1-8396-452a-a38c-017ff3a15e99-13&pdp_ext_f=%7B%22order%22%3A%221%22%2C%22eval%22%3A%221%22%2C%22fromPage%22%3A%22search%22%7D&pdp_npi=6%40dis%21USD%215.20%211.73%21%21%2135.91%2111.93%21%402103110517704109713072570ec0f4%2112000056052263189%21sea%21US%210%21ABX%211%210%21n_tag%3A-29910%3Bd%3A9b04ae%3Bm03_new_user%3A-29895%3BpisId%3A5000000200404592&curPageLogUid=clN74e2x8USc&utparam-url=scene%3Asearch%7Cquery_from%3A%7Cx_object_id%3A1005011611602420%7C_p_origin_prod%3A) |
| 3D Printed Part | 6 | – |
| Breadboard and Jumper Wires | – | – |
| 18650 Battery | 13.88 | [Amazon](https://www.amazon.com/Yuntunele-Rechargeable-Battery-3500mAh-Headlights/dp/B0CWXHTLVT/ref=sr_1_10?crid=2J6G6QW6FBUJR&dib=eyJ2IjoiMSJ9.9ulHbkJK9GJyQjH1pxiEVk5bcpwpcnJaoLFb5Z5auDc62mkM2_6IpRhDZAi_yvnqfeZMaLgUOpJ8EY8EEoMt5L-uzffw-QqfifTo8CuRs3y0rsm5MK-PCiFdcFLa4xdwFGcYy5dCC3biq9Go-UJuhkkmCoIa5H9pYUl2CTGaTkWjzO0l5V-_VjMP7QfgEbjyxUFJyJ3LbbgkABaWx4NXwbAku9Nussu-Ep149j0zS_4Ok45MXop7DVwxK-9_dtjUs7Ni1p-lQ1_dTDjfFhwi26F3xIz92DmP6KmJxfR5C1U.s-Cm3jFqYnSzRL6TsKvZgBUZoc8Jsl8a2U7YDTjyIGw&dib_tag=se&keywords=18650+Battery&qid=1770406772&sprefix=18650+battery%2Caps%2C209&sr=8-10) |
| Battery Shield | 5.37 | [Aliexpress](https://www.aliexpress.us/item/3256806674842292.html?spm=a2g0o.productlist.main.3.777dfFopfFop8w&algo_pvid=c0bf7d60-a24c-4dfe-af7b-6c5eb9d6ed50&algo_exp_id=c0bf7d60-a24c-4dfe-af7b-6c5eb9d6ed50-2&pdp_ext_f=%7B%22order%22%3A%224281%22%2C%22eval%22%3A%221%22%2C%22orig_sl_item_id%22%3A%221005006861157044%22%2C%22orig_item_id%22%3A%221005007538358028%22%2C%22fromPage%22%3A%22search%22%7D&pdp_npi=6%40dis%21USD%215.37%212.69%21%21%2136.98%2118.49%21%402103122117731084433141169e55b0%2112000056017683280%21sea%21US%210%21ABX%211%210%21n_tag%3A-29910%3Bd%3A9b04ae%3Bm03_new_user%3A-29895&curPageLogUid=GkSDyHLI8cRb&utparam-url=scene%3Asearch%7Cquery_from%3A%7Cx_object_id%3A1005006861157044%7C_p_origin_prod%3A1005007538358028) |
