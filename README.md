# tzaar_bot

這是一個針對 [TZAAR 棋盤遊戲](https://en.wikipedia.org/wiki/Tzaar) 開發的 AI 機器人專案。

## 專案簡介
本專案為 [114 程式設計課程] 的期中專案，目標是實作一個能夠進行合法移動並具備基本策略的 TZAAR 遊戲引擎。

## 課程規範 (Project Specification)
本專案根據 [2026PD 課程規範](https://hackmd.io/@2026PD/ryXceLwdWx) 進行開發。

## 已實作功能
- [x] 核心遊戲引擎 (Core Engine)
- [x] 合法步移動邏輯 (Legal Move Generation)
- [x] 隨機移動策略 (Random Move Logic)
- [x] 啟發式評估函數 (Heuristic Evaluation) 



## 學號 (Student ID(s)) : E24146644

## 姓名 (Name(s)) : 何家睿

## 完成/最後更新時間 (Date/Time) : 2026/4/30 00:15 

## 開發與執行環境 (Environment) : 
    - OS: Windows 11 (64-bit)
    - Editor: Visual Studio Code (1.115)
    - Terminal: Cygwin
    - Compiler: g++ (GCC) 13.4.0
    - 語言版本: Compiler Default


## 執行方式 (How to Run) :

    - Cygwin切換到專案資料夾
    - 輸入 g++ -o E24146644 E24146644.cpp
    - 輸入(平台呼叫方式)  ./E24146644 [Color] [Round] [board] [chessLayer] [stepHistory]


## 依賴或注意事項 (Notes/Dependencies) : 

### 頭文件引入<bits/stdc++.h> 用到其中:
        <iostream>
        <fstream>    
        <string>     
        <vector>  
        <ctime>     用於時間戳記
        <sstream>   字串串流切割  
        <algorithm> max, min, sort, swap 用於orderMove()
        <stdexcept> 用於拋出超時例外


    有利用到stoi() (string to integer) 這個函數，C++11 版本才有辦法使用。

