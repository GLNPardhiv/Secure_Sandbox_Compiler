import sys
import json
import requests
import os

from dotenv import load_dotenv
load_dotenv()

API_KEY = os.getenv("API_KEY") 

# Try the v1 endpoint instead of v1beta
API_URL = f"https://generativelanguage.googleapis.com/v1beta/models/gemini-2.5-flash:generateContent?key={API_KEY}"

def analyze_with_gemini(code_content):
    """
    Sends the C++ code to Google Gemini.
    Asks it to act as a Security Auditor.
    """
    
    # The System Prompt: We tell the AI exactly what to look for.
    prompt_text = f"""
    You are a Static Analysis Security Tool (SAST) for a C++ Sandbox.
    Analyze the following user code for malicious intent or resource abuse.

    Specific Risks to flag:
    1. Infinite Loops (while(1), for(;;)) -> Risk: DoS
    2. Fork Bombs (while(1) fork()) -> Risk: System Crash
    3. System Calls (system(), exec(), popen()) -> Risk: RCE
    4. Network Calls (socket, connect) -> Risk: Reverse Shell
    5. File I/O (ofstream, remove) -> Risk: File System Damage

    User Code:
    ```cpp
    {code_content}
    ```

    Respond ONLY with a valid JSON object in this exact format (no markdown formatting):
    {{
        "risk_score": (integer 0-10, where 10 is critical),
        "is_safe": (boolean, false if risk_score > 6),
        "analysis": "(string, brief explanation of the risk)"
    }}
    """

    payload = {
        "contents": [{
            "parts": [{
                "text": prompt_text
            }]
        }]
    }
    
    headers = { "Content-Type": "application/json" }

    try:
        # Updated Line: Wait up to 15 seconds
        response = requests.post(API_URL, headers=headers, json=payload, timeout=15)
        
        if response.status_code != 200:
            return {
                "risk_score": 0, 
                "is_safe": True, 
                "analysis": f"AI Error: API returned {response.status_code}. Response: {response.text}"
            }

        data = response.json()
        
        # Extract the text reply from Gemini
        try:
            ai_text = data['candidates'][0]['content']['parts'][0]['text']
        except (KeyError, IndexError):
             return {
                "risk_score": 0, 
                "is_safe": True, 
                "analysis": "AI Error: Unexpected JSON structure from API."
            }
        
        # Sanitize: Remove markdown code fences if Gemini adds them
        ai_text = ai_text.replace("```json", "").replace("```", "").strip()
        
        return json.loads(ai_text)

    except Exception as e:
        # Fail-open: If internet is down, we let the Sandbox OS limits handle it.
        return {
            "risk_score": 0, 
            "is_safe": True, 
            "analysis": f"AI Connection Failed: {str(e)}"
        }

def main():
    if len(sys.argv) < 2:
        # If no file provided, print error but don't crash
        print(json.dumps({"error": "No file provided"}))
        sys.exit(1)
        
    filepath = sys.argv[1]
    
    try:
        with open(filepath, 'r') as f:
            code_content = f.read()
            
        # Check if user forgot to set key
        if API_KEY == "PASTE_YOUR_API_KEY_HERE" or API_KEY == "":
            print(json.dumps({
                "risk_score": 0,
                "is_safe": True, 
                "analysis": "CONFIGURATION ERROR: API Key is missing in risk_analyzer.py"
            }))
        else:
            result = analyze_with_gemini(code_content)
            print(json.dumps(result))
            
    except FileNotFoundError:
        print(json.dumps({"error": "File not found"}))
    except Exception as e:
        print(json.dumps({"error": str(e)}))

if __name__ == "__main__":
    main()