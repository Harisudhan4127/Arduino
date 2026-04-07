import streamlit as st
import serial
import json
import pandas as pd
import time
import serial.tools.list_ports

st.set_page_config(page_title="Asthma Care Pro", layout="wide")

# Persistent data storage
if 'history' not in st.session_state:
    st.session_state.history = pd.DataFrame(columns=['Time', 'BPM', 'SpO2', 'AQI'])

st.title("🫁 Asthma Care Pro: Live Analytics")

# Sidebar Configuration
st.sidebar.header("Connection Settings")
ports = [p.device for p in serial.tools.list_ports.comports()]
selected_port = st.sidebar.selectbox("Select NodeMCU Port", ports)

if st.sidebar.button("Launch Dashboard"):
    try:
        ser = serial.Serial(selected_port, 115200, timeout=1)
        st.sidebar.success("Linked to Hardware")
        
        # Dashboard Placeholders
        m1, m2, m3, m4 = st.columns(4)
        chart_col, map_col = st.columns([2, 1])
        
        while True:
            line = ser.readline().decode('utf-8').strip()
            if line.startswith('{'):
                data = json.loads(line)
                curr_time = time.strftime("%H:%M:%S")

                # 1. Update Top Metrics
                m1.metric("Heart Rate", f"{data['bpm']} BPM")
                m2.metric("Oxygen", f"{data['spo2']}%")
                m3.metric("Air Quality", data['aqi'])
                m4.metric("Risk Level", data['status'])

                # 2. Update History
                new_entry = pd.DataFrame([[curr_time, data['bpm'], data['spo2'], data['aqi']]], 
                                         columns=['Time', 'BPM', 'SpO2', 'AQI'])
                st.session_state.history = pd.concat([st.session_state.history, new_entry], ignore_index=True).tail(30)

                # 3. Render Charts
                with chart_col:
                    st.subheader("Vital Signs Trend")
                    st.line_chart(st.session_state.history.set_index('Time')[['BPM', 'SpO2']])
                
                # 4. GPS Mapping
                with map_col:
                    st.subheader("Patient Location")
                    map_data = pd.DataFrame({'lat': [data['lat']], 'lon': [data['lng']]})
                    st.map(map_data)
                
                time.sleep(0.1) # Smooth UI refresh
                
    except Exception as e:
        st.sidebar.error(f"Error: {e}")
else:
    st.info("Please connect the NodeMCU via USB and click 'Launch Dashboard'.")