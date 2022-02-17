# HokakuCTR 

A 3DS game plugin (3GX) to dump the RMC communication between 3DS games and NEX. The traffic is dumped to the SD into **pcap** files.

## Usage

1. Install the Luma3DS 3GX plugin loader.
2. Navigate to **luma/plugins** and copy the **.3gx** file as **default.3gx** to load it for all game or place it inside a folder with the game *titleID* you want to use.
3. Open the Rosalina menu and enable the plugin loader.
4. Launch the game to dump traffic from. The screen should flash blue and some text will display on the screen. If it says **Not Ready**, it means this game is not compatible. If it says **Ready**, you can go online to dump the traffic.

The dumps will be placed inside the **HokakuCTR** folder in the SD card root.