package com.example.mqtt_test1;

import androidx.appcompat.app.AppCompatActivity;

import android.annotation.SuppressLint;

import android.content.DialogInterface;

import android.os.Bundle;

import android.os.Handler;

import android.os.Message;

import android.text.method.ScrollingMovementMethod;
import android.view.View;

import android.view.ViewDebug;
import android.view.Window;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;
import com.google.android.material.snackbar.Snackbar;

import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.MqttCallbackExtended;
import org.eclipse.paho.client.mqttv3.MqttClient;
import org.eclipse.paho.client.mqttv3.MqttConnectOptions;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.eclipse.paho.client.mqttv3.MqttMessage;
import org.eclipse.paho.client.mqttv3.persist.MemoryPersistence;

import java.text.DecimalFormat;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.ScheduledExecutorService;

import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;

import java.util.Timer;
import java.util.TimerTask;

public class MainActivity extends AppCompatActivity implements View.OnClickListener {
    private TextView d_mode;
    private TextView d_status;
    private TextView d_distance;
    private TextView d_speed;
    private Button FRONT;
    private Button STOP;
    private Button BACK;
    private Button RETURN;
    private Button kongzhi;
    private Button yushe;
    private Button[] tags = new Button[12];
    private Button tag1;
    private Button tag2;
    private Button tag3;
    private Button tag4;
    private Button tag5;
    private Button tag6;
    private Button tag7;
    private Button tag8;
    private Button tag9;
    private Button tag10;
    private Button tag11;
    private Button tag12;

    private TextView tv_recv;
    boolean model_flag = false;
    boolean front1_flag = false;
    boolean back1_flag = false;
    boolean stop1_flag = false;
    boolean front2_flag = false;
    boolean back2_flag = false;
    boolean stop2_flag = false;
    boolean return_flag = false;
    boolean quityushe_when_moving_flag = false;
    boolean return_when_moving_flag = false;
    private Runnable runnable;
    private String host = "tcp://10wv1pa465244.vicp.fun:12562";
    //private String host = "tcp://192.168.137.34:1883";
    private String userName = "gateway2";
    private String passWord = "hn123456";
    private String mqtt_id = "111111";
    private Handler handler;
    private MqttClient client;
    private String mqtt_sub_topic = "Catstatus";
    private String mqtt_pub_topic = "Catcontrol";
    private MqttConnectOptions options;
    private ScheduledExecutorService scheduler;
    final ExecutorService executorService = new ThreadPoolExecutor(1, 1, 0, TimeUnit.MILLISECONDS,
            new LinkedBlockingQueue<Runnable>());

    @SuppressLint("HandlerLeak")
    //内存泄露忽略
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        //requestWindowFeature(Window.FEATURE_LEFT_ICON);
        setContentView(R.layout.activity_main);
        //getWindow().setFeatureDrawableResource(Window.FEATURE_LEFT_ICON, R.drawable.xiaohui);
        init_view();
        Button_view();
        initTags();
        init();

        startReconnect();
        SendLoraUploadDataCmd();
        //startTimer();
        handler = new Handler() {
            @SuppressLint("SetTextIl8n")
            public void handleMessage(Message msg) {
                super.handleMessage(msg);
                switch (msg.what) {
                    case 1: //开机校验更新回传
                        break;
                    case 2: //反馈回转
                        break;
                    case 3: //MQTT收到消息回传
                        tv_recv.append("\n");
                        System.out.println(msg.obj.toString());
                        datahandler(msg.obj.toString());
                        RefreshUI();
                        break;
                    case 30: //连接失败
                        Toast.makeText(MainActivity.this, "连接失败", Toast.LENGTH_SHORT).show();
                        break;
                    case 31: //连接成功
                        Toast.makeText(MainActivity.this, "连接成功", Toast.LENGTH_SHORT).show();
                        //publishmessageplus(mqtt_pub_topic, "第一个客户端发送的信息");
                        break;
                    default:
                        break;
                }
            }
        };

    }

    public void refreshTextView(String msg) {
        tv_recv.append(msg);
        int offset = tv_recv.getLineCount() * tv_recv.getLineHeight();
        if (offset > tv_recv.getHeight()) {
            tv_recv.scrollTo(0, offset - tv_recv.getHeight());
        }
    }

    String current_tag = "";
    @SuppressLint({"DefaultLocale", "SetTextI18n"})
    public void datahandler(String msg) {
        String data1 = "";
        char[] char1 = msg.toCharArray();

        for (int i = 1; i < 5; i++) {
            data1 = data1 + char1[i];
        }
        current_tag = "" + char1[5] + char1[6];

        String mode = "";
        String status = "";
        String distance = "";
        String speed = "";
        float distance1 = 0;
        int status1 = 0;

        if (char1.length > 7) {
            for (int i = 1; i < 5; i++) {
                mode = mode + char1[i];
                speed = speed + char1[i + 11];
            }
            status = status + char1[5] + char1[6];
            for (int i = 7; i < 12; i++) {
                distance = distance + char1[i];
            }
            distance1 = Float.parseFloat(distance);
            status1 = Integer.parseInt(status);
        }

        float x1 = distance1;
        int x2 = status1;
        double x3 = 0.0; // 使用 double 类型
        String fdis = "";
        String bdis = "";

        DecimalFormat df = new DecimalFormat(".##");
        if(x2 >= 7 && x2 <= 12){
            x3 = (x2 - 7) * 28;
            fdis = df.format(45 + x3 + x1);
            bdis = df.format(45 + x3 - x1);
        }else if(x2 > 0 && x2 < 4){
            x3 = (x2 - 1) * 10;
            fdis = df.format(x3 + x1);
            bdis = df.format(x3 - x1);
        }else if(x2 >= 4 && x2 < 7){
            x3 = (x2 - 4) * 13;
            fdis = df.format(x3 + x1);
            bdis = df.format(x3 - x1);
        }

        switch (data1) {
            case "9911":
                if (!model_flag) {
                    Toast.makeText(MainActivity.this, "已进入控制模式", Toast.LENGTH_SHORT).show();
                    model_flag = true;
                }
                d_mode.setText("控制模式");
                break;
            case "9811":
                if (!model_flag) {
                    Toast.makeText(MainActivity.this, "已进入预设模式", Toast.LENGTH_SHORT).show();
                    model_flag = true;
                }
                d_mode.setText("预设模式");
                break;
            case "9901":
                //refreshTextView("已退出控制模式");
                //Toast.makeText(MainActivity.this, "已退出控制模式", Toast.LENGTH_SHORT).show();
                model_flag = false;

                front1_flag = false;
                stop1_flag = false;
                back1_flag = false;

                front2_flag = false;
                stop2_flag = false;
                back2_flag = false;

                return_flag = false;

                d_mode.setText("");
                d_status.setText("停止");
                d_speed.setText("");

                break;
            case "9801":
                //refreshTextView("已退出预设模式");
                //Toast.makeText(MainActivity.this, "已退出预设模式", Toast.LENGTH_SHORT).show();
                model_flag = false;

                front1_flag = false;
                stop1_flag = false;
                back1_flag = false;

                front2_flag = false;
                stop2_flag = false;
                back2_flag = false;

                return_flag = false;
                d_mode.setText("");
                break;
            case "0000":
                model_flag = false;

                front1_flag = false;
                stop1_flag = false;
                back1_flag = false;

                front2_flag = false;
                stop2_flag = false;
                back2_flag = false;

                return_flag = false;

                d_mode.setText("");
                d_status.setText("停止");
                d_speed.setText("");
                break;
            case "1111":
                d_mode.setText("控制模式");
                d_status.setText("前进");
                //FRONT.setBackgroundColor(getResources().getColor(R.color.purple_500));
                BACK.setEnabled(false);
                BACK.setBackgroundColor(getResources().getColor(R.color.grey));
                d_distance.setText(fdis + " m");
                d_speed.setText(speed + " m/s");
                if (!front2_flag) {
                    Toast.makeText(MainActivity.this, "收到服务器回传前进", Toast.LENGTH_SHORT).show();
                    front2_flag = true;
                    stop2_flag = false;
                    back2_flag = false;
                }
                break;
            case "1010":
                d_mode.setText("控制模式");
                d_status.setText("后退");
                FRONT.setEnabled(false);
                FRONT.setBackgroundColor(getResources().getColor(R.color.grey));
                //BACK.setBackgroundColor(getResources().getColor(R.color.grey));
                d_distance.setText(bdis + " m");
                d_speed.setText(speed + " m/s");
                if (!back2_flag) {
                    Toast.makeText(MainActivity.this, "收到服务器回传后退", Toast.LENGTH_SHORT).show();
                    front2_flag = false;
                    stop2_flag = false;
                    back2_flag = true;
                }
                break;
            case "9090":
                d_status.setText("停止");

                d_speed.setText("");
                if (!stop2_flag) {
                    //Toast.makeText(MainActivity.this, "收到服务器回传停止", Toast.LENGTH_SHORT).show();
                    front2_flag = false;
                    stop2_flag = true;
                    back2_flag = false;
                }
                //Toast.makeText(MainActivity.this, "收到服务器回传停止", Toast.LENGTH_SHORT).show();
                
                if(quityushe_when_moving_flag){
                    quityushe_when_moving_flag = false;
                    yushe.performClick();
                }

                if(return_when_moving_flag){
                    return_when_moving_flag = false;
                    RETURN.performClick();
                }
                return_flag = false; 
                break;
            case "2222":
                d_mode.setText("预设模式");
                d_status.setText("前进");
                d_distance.setText(fdis + " m");
                d_speed.setText(speed + " m/s");
                //Toast.makeText(MainActivity.this, "前进", Toast.LENGTH_SHORT).show();
                break;
            case "2020":
                d_mode.setText("预设模式");
                d_distance.setText(bdis + " m");
                d_speed.setText(speed + " m/s");
                if(return_flag){
                    d_status.setText("召回");
                    //Toast.makeText(MainActivity.this, "召回", Toast.LENGTH_SHORT).show();
                }
                else{
                    d_status.setText("后退");
                    //Toast.makeText(MainActivity.this, "后退", Toast.LENGTH_SHORT).show();
                }
                
                break;
            }


            //当车子回到1号标签处时距离清零
            if(status.equals("01") && distance1 == 0){
                d_distance.setText("0.00m");
            }
//        refreshTextView("状态:" + mode);
//        refreshTextView("  标签号:" + status);
//        refreshTextView("  距离:" + distance);
//        refreshTextView("  速度:" + speed);
//                d_mode.setText(mode);
//                d_status.setText(status);
//                d_distance.setText(distance + " m");
//                    d_speed.setText(speed + " m/s");

    }
    
    public void RefreshUI() {
        String mode = d_mode.getText().toString();
        String status = d_status.getText().toString();
        switch(mode)
        {
            case "控制模式":
                kongzhi.setEnabled(true);
                kongzhi.setBackgroundColor(getResources().getColor(R.color.green1));
                
                yushe.setEnabled(false);
                yushe.setBackgroundColor(getResources().getColor(R.color.grey));
                FRONT.setEnabled(true);
                FRONT.setBackgroundColor(getResources().getColor(R.color.purple_500));
                STOP.setEnabled(true);
                STOP.setBackgroundColor(getResources().getColor(R.color.purple_500));
                BACK.setEnabled(true);
                BACK.setBackgroundColor(getResources().getColor(R.color.purple_500));
                RETURN.setEnabled(false);
                RETURN.setBackgroundColor(getResources().getColor(R.color.grey));
                setTagsClickable(false);
                break;
            case "预设模式":
                yushe.setBackgroundColor(getResources().getColor(R.color.green1));
                kongzhi.setEnabled(false);
                kongzhi.setBackgroundColor(getResources().getColor(R.color.grey));
                yushe.setEnabled(true);
                //yushe.setBackgroundColor(getResources().getColor(R.color.purple_500));
                FRONT.setEnabled(false);
                FRONT.setBackgroundColor(getResources().getColor(R.color.grey));
                STOP.setEnabled(true);
                STOP.setBackgroundColor(getResources().getColor(R.color.purple_500));
                BACK.setEnabled(false);
                BACK.setBackgroundColor(getResources().getColor(R.color.grey));
                RETURN.setEnabled(true);
                RETURN.setBackgroundColor(getResources().getColor(R.color.purple_500));
                setTagsClickable(true);
                break;
            case "":
                kongzhi.setEnabled(true);
                kongzhi.setBackgroundColor(getResources().getColor(R.color.purple_500));
                yushe.setEnabled(true);
                yushe.setBackgroundColor(getResources().getColor(R.color.purple_500));
                FRONT.setEnabled(false);
                FRONT.setBackgroundColor(getResources().getColor(R.color.grey));
                STOP.setEnabled(false);
                STOP.setBackgroundColor(getResources().getColor(R.color.grey));
                BACK.setEnabled(false);
                BACK.setBackgroundColor(getResources().getColor(R.color.grey));
                RETURN.setEnabled(false);
                RETURN.setBackgroundColor(getResources().getColor(R.color.grey));
                setTagsClickable(false);
                break;
        }

        //单轨机所在位置的标签为绿色
        switch (current_tag) 
        {
            case "01":
                tag1.setBackgroundColor(getResources().getColor(R.color.green2, getTheme()));
                tag2.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag3.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag4.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag5.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag6.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag7.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag8.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag9.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag10.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag11.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag12.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                break;
            case "02":
                tag1.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag2.setBackgroundColor(getResources().getColor(R.color.green2, getTheme()));
                tag3.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag4.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag5.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag6.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag7.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag8.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag9.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag10.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag11.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag12.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                break;
            case "03":
                tag1.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag2.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag3.setBackgroundColor(getResources().getColor(R.color.green2, getTheme()));
                tag4.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag5.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag6.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag7.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag8.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag9.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag10.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag11.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag12.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                break;
            case "04":
                tag1.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag2.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag3.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag4.setBackgroundColor(getResources().getColor(R.color.green2, getTheme()));
                tag5.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag6.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag7.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag8.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag9.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag10.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag11.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag12.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                break;
            case "05":
                tag1.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag2.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag3.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag4.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag5.setBackgroundColor(getResources().getColor(R.color.green2, getTheme()));
                tag6.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag7.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag8.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag9.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag10.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag11.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag12.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                break;
            case "06":
                tag1.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag2.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag3.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag4.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag5.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag6.setBackgroundColor(getResources().getColor(R.color.green2, getTheme()));
                tag7.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag8.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag9.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag10.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag11.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag12.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                break;
            case "07":
                tag1.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag2.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag3.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag4.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag5.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag6.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag7.setBackgroundColor(getResources().getColor(R.color.green2, getTheme()));
                tag8.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag9.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag10.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag11.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag12.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                break;
            case "08":
                tag1.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag2.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag3.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag4.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag5.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag6.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag7.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag8.setBackgroundColor(getResources().getColor(R.color.green2, getTheme()));
                tag9.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag10.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag11.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag12.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                break;
            case "09":
                tag1.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag2.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag3.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag4.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag5.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag6.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag7.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag8.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag9.setBackgroundColor(getResources().getColor(R.color.green2, getTheme()));
                tag10.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag11.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag12.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                break;
            case "10":
                tag1.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag2.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag3.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag4.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag5.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag6.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag7.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag8.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag9.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag10.setBackgroundColor(getResources().getColor(R.color.green2, getTheme()));
                tag11.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag12.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                break;
            case "11":
                tag1.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag2.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag3.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag4.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag5.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag6.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag7.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag8.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag9.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag10.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag11.setBackgroundColor(getResources().getColor(R.color.green2, getTheme()));
                tag12.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                break;
            case "12":
                tag1.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag2.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag3.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag4.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag5.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag6.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag7.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag8.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag9.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag10.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag11.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag12.setBackgroundColor(getResources().getColor(R.color.green2, getTheme()));
                break;
            default:
                tag1.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag2.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag3.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag4.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag5.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag6.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag7.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag8.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag9.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag10.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag11.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                tag12.setBackgroundColor(getResources().getColor(R.color.red, getTheme()));
                break;
        }

        setoneTagClickable(current_tag,false);

        switch(status)
        {
            case "前进":
                setTagsClickable(false);
                break;
            case "后退":
                setTagsClickable(false);
                break;
            case "召回":
                setTagsClickable(false);
                break;
            case "停止":
                if(mode.equals("预设模式")){
                    setTagsClickable(true);
                }
                
                break;
        }
        
    }


    @SuppressLint("NonConstantResourceId")
    @Override
    public void onClick(View v) {
        String status = d_status.getText().toString();
        switch (v.getId())           //通过id判断是哪个按钮被点击
        {
            case R.id.kongzhi:
                publishmessageplus(mqtt_pub_topic, "A99\r\n");
                kongzhi.setEnabled(true);
                yushe.setEnabled(false);
                FRONT.setEnabled(true);
                STOP.setEnabled(true);
                BACK.setEnabled(true);
                RETURN.setEnabled(false);
                setTagsClickable(false);
                break;
            case R.id.yushe:
                
                if(status.equals("后退")||status.equals("前进")||status.equals("召回")){
                    publishmessageplus(mqtt_pub_topic, "A90\r\n");
                    quityushe_when_moving_flag = true;
                }
                else
                    publishmessageplus(mqtt_pub_topic, "A98\r\n");
                kongzhi.setEnabled(false);
                yushe.setEnabled(true);
                FRONT.setEnabled(false);
                STOP.setEnabled(true);
                BACK.setEnabled(false);
                RETURN.setEnabled(true);

                setTagsClickable(true);
                break;
            case R.id.FRONT:
                publishmessageplus(mqtt_pub_topic, "A31\r\n");
                //STOP.setEnabled(true);
                BACK.setEnabled(false);
                if (!front1_flag) {
                    Toast.makeText(MainActivity.this, "点击前进", Toast.LENGTH_SHORT).show();
                    front1_flag = true;
                    stop1_flag = false;
                    back1_flag = false;
                }
                break;
            case R.id.STOP:
                if(d_status.getText().equals("")||d_status.getText().equals("停止"))
                    break;
                publishmessageplus(mqtt_pub_topic, "A90\r\n");
                if(d_mode.getText().equals("控制模式")){
                    FRONT.setEnabled(true);
                    BACK.setEnabled(true);
                }
                if (!stop1_flag) {
                    Toast.makeText(MainActivity.this, "点击停止", Toast.LENGTH_SHORT).show();
                    front1_flag = false;
                    stop1_flag = true;
                    back1_flag = false;
                }
                break;
            case R.id.BACK:
                publishmessageplus(mqtt_pub_topic, "A30\r\n");
                //STOP.setEnabled(true);
                FRONT.setEnabled(false);
                if (!back1_flag) {
                    Toast.makeText(MainActivity.this, "点击后退", Toast.LENGTH_SHORT).show();
                    front1_flag = false;
                    stop1_flag = false;
                    back1_flag = true;
                }
                break;
            case R.id.RETURN:
                if(status.equals("后退")||status.equals("前进")){
                    publishmessageplus(mqtt_pub_topic, "A90\r\n");
                    return_when_moving_flag = true;
                }
                else if(!status.equals("召回"))
                    publishmessageplus(mqtt_pub_topic, "Atag01\r\n");
                if (!return_flag&&!return_when_moving_flag) {
                    Toast.makeText(MainActivity.this, "点击召回", Toast.LENGTH_SHORT).show();
                    return_flag = true;
                }
                break;
            case R.id.tag1:
                publishmessageplus(mqtt_pub_topic, "Atag01\r\n");
                break;
            case R.id.tag2:
                publishmessageplus(mqtt_pub_topic, "Atag02\r\n");
                break;
            case R.id.tag3:
                publishmessageplus(mqtt_pub_topic, "Atag03\r\n");
                break;
            case R.id.tag4:
                publishmessageplus(mqtt_pub_topic, "Atag04\r\n");
                break;
            case R.id.tag5:
                publishmessageplus(mqtt_pub_topic, "Atag05\r\n");
                break;
            case R.id.tag6:
                publishmessageplus(mqtt_pub_topic, "Atag06\r\n");
                break;
            case R.id.tag7:
                publishmessageplus(mqtt_pub_topic, "Atag07\r\n");
                break;
            case R.id.tag8:
                publishmessageplus(mqtt_pub_topic, "Atag08\r\n");
                break;
            case R.id.tag9:
                publishmessageplus(mqtt_pub_topic, "Atag09\r\n");
                break;
            case R.id.tag10:
                publishmessageplus(mqtt_pub_topic, "Atag10\r\n");
                break;
            case R.id.tag11:
                publishmessageplus(mqtt_pub_topic, "Atag11\r\n");
                break;
            case R.id.tag12:
                publishmessageplus(mqtt_pub_topic, "Atag12\r\n");
                break;
        }
    }

    private void Button_view() {
        kongzhi.setOnClickListener(this);
        yushe.setOnClickListener(this);
        FRONT.setOnClickListener(this);
        STOP.setOnClickListener(this);
        BACK.setOnClickListener(this);
        RETURN.setOnClickListener(this);
        tag1.setOnClickListener(this);
        tag2.setOnClickListener(this);
        tag3.setOnClickListener(this);
        tag4.setOnClickListener(this);
        tag5.setOnClickListener(this);
        tag6.setOnClickListener(this);
        tag7.setOnClickListener(this);
        tag8.setOnClickListener(this);
        tag9.setOnClickListener(this);
        tag10.setOnClickListener(this);
        tag11.setOnClickListener(this);
        tag12.setOnClickListener(this);
    }

    private void init_view() {
        FRONT = findViewById(R.id.FRONT);
        STOP = findViewById(R.id.STOP);
        BACK = findViewById(R.id.BACK);
        RETURN = findViewById(R.id.RETURN);
        kongzhi = findViewById(R.id.kongzhi);
        yushe = findViewById(R.id.yushe);
        tag1 = findViewById(R.id.tag1);
        tag2 = findViewById(R.id.tag2);
        tag3 = findViewById(R.id.tag3);
        tag4 = findViewById(R.id.tag4);
        tag5 = findViewById(R.id.tag5);
        tag6 = findViewById(R.id.tag6);
        tag7 = findViewById(R.id.tag7);
        tag8 = findViewById(R.id.tag8);
        tag9 = findViewById(R.id.tag9);
        tag10 = findViewById(R.id.tag10);
        tag11 = findViewById(R.id.tag11);
        tag12 = findViewById(R.id.tag12);

        d_mode = findViewById(R.id.d_mode);
        d_status = findViewById(R.id.d_status);
        d_distance = findViewById(R.id.d_distance);
        d_speed = findViewById(R.id.d_speed);

        tv_recv = (TextView) findViewById(R.id.tv_recv);
        //tv_recv.setMovementMethod(new ScrollingMovementMethod());
    }

    // 在初始化时将 tag1 到 tag12 赋值给数组
    public void initTags() {
        tags[0] = tag1;
        tags[1] = tag2;
        tags[2] = tag3;
        tags[3] = tag4;
        tags[4] = tag5;
        tags[5] = tag6;
        tags[6] = tag7;
        tags[7] = tag8;
        tags[8] = tag9;
        tags[9] = tag10;
        tags[10] = tag11;
        tags[11] = tag12;
    }

    public void setTagsClickable(boolean isClickable) {
        for (Button tag : tags) {
            if (tag != null) {
                tag.setClickable(isClickable);
            }
        }
    }

    public void setoneTagClickable(String tagIndices, boolean isClickable) {
        // 遍历字符串中的每个字符
        int index = Integer.parseInt(tagIndices)-1;
        if (tags[index] != null) {
            // 设置对应 tag 的点击状态
            tags[index].setClickable(isClickable);
        }
        
    }

    private void init() {
        try {
            //host为主机名，test为clientid即连接MQTT的客户端ID，一般以客户端唯一标识符表示，MemoryPersistence设置clientid的保存形式，默认为以内存保存
            client = new MqttClient(host, mqtt_id,
                    new MemoryPersistence());
            //MQTT的连接设置
            options = new MqttConnectOptions();
            //设置是否清空session,这里如果设置为false表示服务器会保留客户端的连接记录，这里设置为true表示每次连接到服务器都以新的身份连接
            options.setCleanSession(false);
            //设置连接的用户名
            options.setUserName(userName);
            //设置连接的密码
            options.setPassword(passWord.toCharArray());
            // 设置超时时间 单位为秒
            options.setConnectionTimeout(10);
            // 设置会话心跳时间 单位为秒 服务器会每隔1.5*20秒的时间向客户端发送个消息判断客户端是否在线，但这个方法并没有重连的机制
            options.setKeepAliveInterval(50);
            //是否自动重连
            options.setAutomaticReconnect(true);

            //设置回调
            client.setCallback(new MqttCallbackExtended() {
                @Override
                public void connectionLost(Throwable cause) {
                    //连接丢失后，一般在这里面进行重连
                    System.out.println("connectionLost----------");
                }

                @Override
                public void deliveryComplete(IMqttDeliveryToken token) {
                    //publish后会执行到这里
                    System.out.println("deliveryComplete---------"
                            + token.isComplete());
                }

                @Override
                public void messageArrived(String topicName, MqttMessage message) {
                    //subscribe后得到的消息会执行到这里面
                    System.out.println("messageArrived----------");
                    Message msg = new Message();
                    msg.what = 3;
                    msg.obj = message.toString();
                    handler.sendMessage(msg);
                }

                @Override
                public void connectComplete(boolean reconnect, String serverURI) {
                    /*
                     * 客户端连接成功后就需要尽快订阅需要的Topic。
                     */
                    System.out.println("connect success");
                    // 连接成功后订阅主题
                    try {
                        client.subscribe(mqtt_sub_topic, 2);
                        Message msg = new Message();
                        msg.what = 31;
                        handler.sendMessage(msg);
                        //发布Lora设备上报设备信息的指令
                        publishmessageplus(mqtt_pub_topic, "A00\r\n");

                    } catch (MqttException e) {
                        e.printStackTrace();
                    }

                }
            });
            //连接MQTT服务器
            Mqtt_connect();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private void Mqtt_connect() {
        try {
            client.connect(options);
        } catch (MqttException  e) {
            e.printStackTrace();
            Message msg = new Message();
            msg.what = 30;
            handler.sendMessage(msg);
        }
    }

    private void startReconnect() {
        scheduler = Executors.newSingleThreadScheduledExecutor();
        scheduler.scheduleWithFixedDelay(new Runnable() {
            @Override
            public void run() {
                if (!client.isConnected()) {
                    Mqtt_connect();
                }
            }
        }, 0 * 1000, 10 * 1000, TimeUnit.MILLISECONDS);
    }

    private void SendLoraUploadDataCmd() {
        scheduler = Executors.newSingleThreadScheduledExecutor();
        scheduler.scheduleWithFixedDelay(new Runnable() {
            @Override
            public void run() {
                publishmessageplus(mqtt_pub_topic, "A00\r\n");
            }
        }, 5, 5, TimeUnit.SECONDS);
    }


    private void publishmessageplus(String topic, String message2) {
        if (client == null || !client.isConnected()) {
            return;
        }
        MqttMessage message = new MqttMessage();
        message.setPayload(message2.getBytes());
        try {
            client.publish(topic, message);
        } catch (MqttException e) {
            e.printStackTrace();
            startReconnect();
        }
    }
}