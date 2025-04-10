function updateForm() {
    const mode = document.getElementById("mode").value;
    const inputs = document.getElementById("inputs");
    document.querySelectorAll(".circuit").forEach(c => c.style.display = "none");
    document.getElementById("resultados").innerHTML = "";
  
    if (mode === "astable") {
      inputs.innerHTML = `
        <label>R1 (Ohm): <input type="number" id="R1" oninput="calcularAstable()"></label>
        <label>R2 (Ohm): <input type="number" id="R2" oninput="calcularAstable()"></label>
        <label>C (Faradios): <input type="number" id="C" step="0.000001" oninput="calcularAstable()"></label>
      `;
      document.getElementById("img_astable").style.display = "block";
    } else if (mode === "monostable") {
      inputs.innerHTML = `
        <label>R (Ohm): <input type="number" id="Rm" oninput="calcularMonostable()"></label>
        <label>C (Faradios): <input type="number" id="Cm" step="0.000001" oninput="calcularMonostable()"></label>
      `;
      document.getElementById("img_monostable").style.display = "block";
    } else if (mode === "pwm") {
      inputs.innerHTML = `<p>Conecte una resistencia variable al pin analógico.</p>`;
      document.getElementById("img_pwm").style.display = "block";
    }
  }
  
  function enviar() {
    const mode = document.getElementById("mode").value;
    let url = `/command?mode=${mode}`;
    if (mode === "astable") {
      url += `&R1=${document.getElementById("R1").value}`;
      url += `&R2=${document.getElementById("R2").value}`;
      url += `&C=${document.getElementById("C").value}`;
    } else if (mode === "monostable") {
      url += `&R=${document.getElementById("Rm").value}`;
      url += `&C=${document.getElementById("Cm").value}`;
    }
    fetch(url)
      .then(response => response.text())
      .then(text => alert(text));
  }
  
  function detener() {
    fetch("/command?mode=stop")
      .then(response => response.text())
      .then(text => alert("Detenido"));
  }
  
  function calcularAstable() {
    const R1 = parseFloat(document.getElementById("R1").value);
    const R2 = parseFloat(document.getElementById("R2").value);
    const C = parseFloat(document.getElementById("C").value);
  
    if (isNaN(R1) || isNaN(R2) || isNaN(C) || R1 <= 0 || R2 <= 0 || C <= 0) {
      document.getElementById("resultados").innerHTML = "";
      return;
    }
  
    const f = 1.44 / ((R1 + 2 * R2) * C);
    const T = 1 / f;
    const D = ((R1 + R2) / (R1 + 2 * R2)) * 100;
  
    document.getElementById("resultados").innerHTML = `
      <p>Frecuencia: ${f.toFixed(2)} Hz</p>
      <p>Periodo: ${T.toFixed(4)} s</p>
      <p>Duty Cycle: ${D.toFixed(2)}%</p>
    `;
  }
  
  function calcularMonostable() {
    const R = parseFloat(document.getElementById("Rm").value);
    const C = parseFloat(document.getElementById("Cm").value);
  
    if (isNaN(R) || isNaN(C) || R <= 0 || C <= 0) {
      document.getElementById("resultados").innerHTML = "";
      return;
    }
  
    const T = 1.1 * R * C; // en segundos
    const Tms = T * 1000;  // en milisegundos
  
    document.getElementById("resultados").innerHTML = `
      <p>Tiempo del pulso: ${T.toFixed(4)} s (${Tms.toFixed(2)} ms)</p>
    `;
  }
  
  window.onload = updateForm;
  