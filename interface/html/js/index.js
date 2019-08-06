
function insertButtons(data)
{
    var buttonDiv = document.getElementById("buttons");
    for (let i = 0; i < data.length; i++) {
        var btn = document.createElement("a");
        btn.setAttribute("href", "execute?" + data[i].name);
        btn.setAttribute("class", "btn btn-primary btn-block");
        btn.innerHTML = data[i].display_name;
        btn.style.fontWeight = 'bold';
        buttonDiv.appendChild(btn);
    }
}

function addButtons()
{
    $.getJSON("scripts.json", insertButtons);
}
