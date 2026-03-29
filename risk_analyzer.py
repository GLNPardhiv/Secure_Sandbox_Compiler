import sys
import json
import requests
import os

from dotenv import load_dotenv
load_dotenv()

# --- CONFIGURATION ---

API_KEY = os.getenv("API_KEY")

API_URL = f"https://generativelanguage.googleapis.com/v1beta/models/gemini-2.5-flash:generateContent?key={API_KEY}"

# --- GEMINI API CALLER ---
def call_gemini(prompt):
    payload = { "contents": [{ "parts": [{ "text": prompt }] }] }
    headers = { "Content-Type": "application/json" }
    try:
        # Timeout increased to 30s for stability
        response = requests.post(API_URL, headers=headers, json=payload, timeout=30)
        if response.status_code != 200:
            return f"AI Error: {response.status_code}"
        
        data = response.json()
        if 'candidates' not in data:
            return "AI returned no content."
            
        text = data['candidates'][0]['content']['parts'][0]['text']
        # Clean up markdown if any slips through
        return text.replace("```json", "").replace("```", "").strip()
    except Exception as e:
        return f"AI Connection Failed: {str(e)}"

# --- LOGIC FUNCTIONS ---

def analyze_risk(code):
    prompt = f"""
    You are a Security Auditor. Analyze this C++ code for malicious intent (Fork bombs, system calls, network access).
    Respond ONLY with JSON: {{ "risk_score": 0-10, "is_safe": boolean, "analysis": "reason" }}
    Code:
    {code}
    """
    return call_gemini(prompt)

def explain_compile_error(code, error_msg):
    prompt = f"""
    You are a C++ Error Analyzer.
    
    Source Code:
    {code}
    
    Compiler Error:
    {error_msg}
    
    Task: Identify the syntax error in ONE sentence.
    Do NOT explain how to fix it. Do NOT use bullet points.
    """
    return call_gemini(prompt)

def explain_runtime_error(code, signal_code):
    reasons = {
        "11": "Segmentation Fault",
        "8": "Floating Point Exception",
        "9": "Killed (Memory Limit)",
        "31": "Bad System Call (Sandbox Violation)",
        "6": "Aborted"
    }
    reason = reasons.get(signal_code, f"Signal {signal_code}")
    
    prompt = f"""
    The C++ code crashed with: {reason}.
    
    Source Code:
    {code}
    
    Task: State the cause of the crash in ONE sentence. 
    Mention the specific line or variable responsible.
    Do NOT explain how to fix it. Do NOT use bullet points.
    """
    return call_gemini(prompt)

# --- MAIN DRIVER ---
def main():
    if len(sys.argv) < 3:
        print(json.dumps({"error": "Usage: python risk_analyzer.py <mode> <file> [extra_arg]"}))
        sys.exit(1)

    mode = sys.argv[1]
    filepath = sys.argv[2]
    
    try:
        with open(filepath, 'r') as f:
            code_content = f.read()
    except:
        code_content = ""

    if mode == "analyze":
        print(analyze_risk(code_content))
    elif mode == "compile_error":
        error_log_path = sys.argv[3]
        try:
            with open(error_log_path, 'r') as ef: error_msg = ef.read()
            # Send first 2000 chars of error log
            print(explain_compile_error(code_content, error_msg[:2000]))
        except: print("Error reading log")
    elif mode == "runtime_error":
        signal_num = sys.argv[3]
        print(explain_runtime_error(code_content, signal_num))

if __name__ == "__main__":
    main()