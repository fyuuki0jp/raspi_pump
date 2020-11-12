var React = require('react');
var ReactDOM = require('react-dom');
const { getMuiTheme, MuiThemeProvider } = require('material-ui/styles');
const { RaisedButton, AppBar, Drawer, MenuItem, Divider,FlatButton, TextField, SelectField,Slider,LinearProgress, Paper, DropDownMenu, Table, TableHeader, TableBody, TableRow, TableHeaderColumn, TableRowColumn, Stepper, Step, StepLabel,Dialog } = require('material-ui');
var socket = io.connect();
var player;
var f = 0;
var container;
var img = new Image(320,240);
socket.on('connect', function (msg) {
});

class View extends React.Component
{
    constructor(props)
    {
        super(props);
        var conf = this.props.config;
        this.state = {
            frame:"",
            open:false,
            time:parseInt(this.props.config.Before)+10,
            imgURL:"",
            config:conf,
            view:this.props.config.useH264
        }
        this.Triger = this.Triger.bind(this);
        this.RecvFrame = this.RecvFrame.bind(this);
        this.FPS = this.FPS.bind(this);
        this.Edge = this.Edge.bind(this);
        this.Mode = this.Mode.bind(this);
        this.Path = this.Path.bind(this);
        this.User = this.User.bind(this);
        this.Pass = this.Pass.bind(this);
        this.Save = this.Save.bind(this);
        this.Rec = this.Rec.bind(this);
        this.Live = this.Live.bind(this);
        this.Timer = this.Timer.bind(this);
        this.SaveTime = this.SaveTime.bind(this);
        this.GetImage = this.GetImage.bind(this);
        this.Use = this.Use.bind(this);
    }
    Triger()
    {
        socket.emit("command","triger");
    }
    RecvFrame(frame)
    {
        f++;
        player.decode(new Uint8Array(frame),{frame:f});
    }
    GetImage(data)
    {
        f++;
        this.setState({imgURL:"data:image/jpeg;base64,"+data});
    }
    Timer()
    {
        this.setState({frame:f+"FPS",time:(this.state.open==true) ? this.state.time-1:this.state.time,open:(this.state.open==true&&(this.state.time-1)>=0) ? true:false});
        f = 0;
    }
    componentDidMount()
    {
        player = new Player({
            useWorker: true,
            workerFile:"worker/Decoder.js",
            webgl:true,
            size:{ width:this.props.width, height:this.props.height}
        });
        player.broadwayOnPictureDecoded=function(){
            console.log("_broadwayOnPictureDecoded")
        }
        player.broadwayOnHeadersDecoded=function(){
            console.log("_broadwayOnHeadersDecoded")
        }
        socket.on("jpeg",this.GetImage);
        socket.on("h264",this.RecvFrame);
        if(this.props.config.useH264 == "ON"){
            container=document.getElementById("canvas_container_id");
 	        container.appendChild(player.canvas);
        }
        setInterval(this.Timer,1000);
    }
    FPS(event,index,value)
    {
        var newConf = this.state.config;
        newConf.fps = value;
        this.setState({config:newConf});
    }
    Mode(event,index,value)
    {
        var newConf = this.state.config;
        newConf.isRec = value;
        this.setState({config:newConf});

    }
    Use(event,index,value)
    {
        var newConf = this.state.config;
        newConf.useH264 = value;
        if(value=="OFF")
            newConf.fps = "30";
        this.setState({config:newConf});
    }
    Edge(event,index,value)
    {
        var newConf = this.state.config;
        newConf.isRising = value;
        this.setState({config:newConf});

    }
    Path(event,index,value)
    {
        var newConf = this.state.config;
        newConf.SavePath = value;
        this.setState({config:newConf});

    }
    Pass(event,index,value)
    {
        var newConf = this.state.config;
        newConf.LoginPass = value;
        this.props.onChange(newConf);
    }
    User(event,index,value)
    {
        var newConf = this.state.config;
        newConf.LoginUser = value;
        this.setState({config:newConf});

    }
    SaveTime(event,value)
    {
        var newConf = this.state.config;
        newConf.Before = value.toString();
        newConf.After = (60-value).toString();
        this.setState({time:value+10,config:newConf});
    }
    Rec()
    {
        socket.emit("command","check");
        this.setState({open:true});
    }
    Live()
    {
        f = 0;
        socket.emit("command","live");
    }
    Save()
    {
        var send = JSON.stringify(this.state.config);
        this.props.onChange(this.state.config);
        socket.emit("config",send);
        
    }
    render()
    {
        var v,v2;
        if(this.state.view == "OFF")
        {
            v = <div id="canvas_container_id">カメラ映像 ({this.state.frame})<br/><img src={this.state.imgURL}/></div>
        }
        else
        {
            v = <div id="canvas_container_id">カメラ映像 ({this.state.frame})<br/></div>
        }
        if(this.state.config.useH264 == "OFF")
        {
            v2 = <DropDownMenu value={this.state.config.fps} onChange={this.FPS}><MenuItem value={"30"} primaryText="30fps" /></DropDownMenu>
        }
        else
        {
            v2 = <DropDownMenu value={this.state.config.fps} onChange={this.FPS}><MenuItem value={"30"} primaryText="30fps" /><MenuItem value={"60"} primaryText="60fps" /><MenuItem value={"90"} primaryText="90fps" /></DropDownMenu>
        }
        return (
            <MuiThemeProvider muiTheme={getMuiTheme()}>
                <br/>
                <RaisedButton label="記録開始" primary={true} onClick={this.Rec}/>
                <RaisedButton label="記録停止" secondary={true} onClick={this.Live}/>
                <RaisedButton label="トリガーテスト" onClick={this.Triger} style={{heihgt:"15px"}}/><br/>
                {v}
                <div id="cofig">
                    FPS設定 : 
                    {v2}<br/>
                    トリガー設定 : 
                    <DropDownMenu value={this.state.config.isRising} onChange={this.Edge}>
                        <MenuItem value={"ON"} primaryText="立ち上がり" />
                        <MenuItem value={"OFF"} primaryText="立ち下がり" />
                    </DropDownMenu><br/>
                    起動時動作設定 : 
                    <DropDownMenu value={this.state.config.isRec} onChange={this.Mode}>
                        <MenuItem value={"ON"} primaryText="記録する" />
                        <MenuItem value={"OFF"} primaryText="記録しない" />
                    </DropDownMenu><br/>
                    {/*保存モード : 
                    <DropDownMenu value={this.state.config.useH264} onChange={this.Use}>
                        <MenuItem value={"ON"} primaryText="H264" />
                        <MenuItem value={"OFF"} primaryText="MJPEG" />
                    </DropDownMenu>*/}
                    <br/><br/>
                    トリガー位置設定
                    <Slider min={0} max={60} step={1} value={this.state.config.Before} onChange={this.SaveTime} style={{width:"400px",margin:"0px"}} sliderStyle={{"margin-bottom":"10px","margin-top":"10px"}}/>
                    トリガー前 : {this.state.config.Before}秒 トリガー後 : {this.state.config.After}秒<br/><br/>
                    保存先設定 : <TextField value={this.state.config.SavePath} onChange={this.Path}/><br/>
                    ユーザー : <TextField value={this.state.config.LoginUser} onChange={this.User}/><br/>
                    パスワード : <TextField value={this.state.config.LoginPass} onChange={this.Pass}/><br/>
                    <Divider />
                    <RaisedButton label="設定保存" onClick={this.Save}/>
                </div>
                <div id="info">
                    <Dialog open={this.state.open} modal={false}>
                        バッファリング中です。あと{this.state.time}秒
                        <LinearProgress mode="determinate" value={parseInt(this.state.config.Before)+10-this.state.time} min={0} max={parseInt(this.state.config.Before)+10}/>
                    </Dialog>
                </div>
            </MuiThemeProvider>
        )
    }
}

class List extends React.Component
{
    constructor(props)
    {
        super(props);
    }
    componentDidMount()
    {
    }
    render()
    {
        var list = [];
        const style={
            width:"50%",
            display:"inline-table"
        };
        var r = this.props.src;

        r.reverse().forEach(function(v){
            var seg = v.split("/");
            var year = seg[4];
            var mon = seg[5];
            var day = seg[6];
            var hour = seg[7];
            var min = seg[8].substr(0,2);
            var sec = seg[8].substr(2,2);
            list.push(<div style={style}>{year+"年"+mon+"月"+day+"日 "+hour+"時"+min+"分"+sec+"秒"}<br/><video src={v} controls preload="metadata"></video></div>);
        });
        return(
            <MuiThemeProvider muiTheme={getMuiTheme()}>
                動画リスト<br/>
                {list}
            </MuiThemeProvider>
        )
    }
}

class Loading extends React.Component {
    constructor(props) {
        super(props);
        this.state = {
            loadimg: "/img/loading.gif"
        }
    }
    render() {
        return (
            <div id="main">
                <h1> Loadig...</h1><br />
                <img src={this.state.loadimg} />
            </div>
        )
    }
}

class App extends React.Component {
    constructor(props) {
        super(props);
        localStorage.clear();
        this.state = {
            open: false,
            imgURL:"",
            imgList:[],
            config:{},
            mode:0,
            check:{lisence:false,hwcode:""},
            swcode:""
        }
        this.Triger = this.Triger.bind(this);
        this.GetImage = this.GetImage.bind(this);
        this.GetList = this.GetList.bind(this);
        this.OpenMenu = this.OpenMenu.bind(this);
        this.HideMenu = this.HideMenu.bind(this);
        this.Main = this.Main.bind(this);
        this.Camera = this.Camera.bind(this);
        this.GetConfig = this.GetConfig.bind(this);
        this.Update = this.Update.bind(this);
        this.GetLicense = this.GetLicense.bind(this);
        this.Authentication = this.Authentication.bind(this);
        this.swUpdate = this.swUpdate.bind(this);
    }
    Triger()
    {
        socket.emit("command","triger");
    }
    GetImage(data)
    {
        this.setState({imgURL:"data:image/jpeg;base64,"+data});
    }
    GetList(data)
    {
        this.setState({imgList:this.state.imgList.concat([data])});
    }
    GetConfig(data)
    {
        console.log(JSON.parse(data));
        this.setState({config:JSON.parse(data)});
    }
    Debug(data)
    {
        console.log("recv to : "+data);
    }
    OpenMenu()
    {
        this.setState({open:true});
    }
    HideMenu()
    {
        this.setState({open:false});
    }
    Main()
    {
        this.setState({open:false,mode:0});
    }
    Camera()
    {
        this.setState({open:false,mode:1});
    }
    GetLicense(data)
    {
        this.setState({check:JSON.parse(data)});
    }
    componentDidMount()
    {
        socket.on("getShot",this.GetList);
        socket.on("config",this.GetConfig);
        socket.on("debug",this.Debug);
        socket.on("license",this.GetLicense);
        socket.emit("request","filelistconfig");
    }
    Update(config)
    {
        this.setState({config:config});
    }
    Authentication()
    {
        console.log("swcode : "+this.state.swcode);
        socket.emit("license",this.state.swcode);
    }
    Cancel()
    {

    }
    swUpdate(event,value)
    {
        this.setState({swcode:value});
    }
    render() {
        var v = <List src={this.state.imgList}/>
        const actions = [
            <FlatButton
              label="キャンセル"
              primary={true}
              disabled={true}
              onClick={this.Cancel}
            />,
            <FlatButton
              label="認証"
              primary={true}
              onClick={this.Authentication}
            />,
          ];
        if(this.state.mode==1)
            v = <View width={320} height={180} config={this.state.config} onChange={this.Update} imgURL={this.state.imgURL}/>
        return (
            <MuiThemeProvider muiTheme={getMuiTheme()}>
                <div id="menu">
                    <AppBar
                        title={"監視カメラ"}
                        iconClassNameRight="muidocs-icon-navigation-expand-more"
                        onLeftIconButtonClick={this.OpenMenu}
                    />
                    <Drawer open={this.state.open} width="30%" docked={true}>
                        <MenuItem onClick={this.Main}>一覧画面</MenuItem>
                        <MenuItem onClick={this.Camera}>プレビュー画面</MenuItem>
                    </Drawer>
                </div>
                <div id="main" onClick={this.HideMenu}>
                    {v}
                </div>
                <div id="info">
                    <Dialog open={this.state.check.lisence==false} modal={true} actions={actions}>
                        ライセンス認証して下さい。<br/>
                        HWKey : {this.state.check.hwcode}<br/>
                        <TextField
                            floatingLabelText="Please SoftWare Key"
                            floatingLabelFixed={true}
                            value={this.state.swcode} onChange={this.swUpdate}
                        />
                    </Dialog>
                </div>
            </MuiThemeProvider>
        )
    }
}
ReactDOM.render(<App />, document.getElementById('app'));