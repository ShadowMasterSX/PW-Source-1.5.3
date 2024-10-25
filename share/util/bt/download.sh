cd /usr/local/BitTorrent-4.1.2

./btdownloadheadless.py --max_upload_rate 0 --minport 26881 --maxport 26999 --socket_timeout -1 --display_interval 3600 --ip 221.192.136.10 --save_as ./download/WMSJYL ./download/WMSJYL.torrent 1>>log.download.1 2>>log.download.2 &

cd -

