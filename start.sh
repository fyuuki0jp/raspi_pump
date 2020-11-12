#一応
sudo systemctl daemon-reload
sudo service stop apache2

#applicationn start
cd /var/work/Release
sudo npm start&
