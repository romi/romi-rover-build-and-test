
class Selector
{
    constructor(callback, classname, options) {
        this.callback = callback;
        this.element = document.createElement('select');
        this.element.className = classname;
        this.element.addEventListener('change', (e) => { this.updateValue(e); });
        this.makeOptions(options);
        this.value = null;
    }

    makeOptions(options) {
        for (const option of options) {
            let element = document.createElement('option');
            element.value = option.value;
            element.label = option.label;
            if (option.selected) {
                element.selected = true;
                this.value = option.value;
            }
            this.element.appendChild(element);
        }
    }

    updateValue(e) {
        this.value = e.target.value;
        this.callback(this);
    }
}

class TextField
{
    constructor(callback, classname, value, length) {
        this.callback = callback;
        this.value = value;
        this.element = document.createElement('input');
        this.element.type = 'text';
        this.element.value = value;
        this.element.size = length;
        this.element.className = classname;
        this.element.addEventListener('change', (e) => { this.updateValue(e); });
    }

    updateValue(e) {
        this.value = e.target.value;
        this.callback(this);
    }
}

class Checkbox
{
    constructor(callback, classname, value) {
        this.callback = callback;
        this.element = document.createElement('input');
        this.element.type = 'checkbox';
        this.element.checked = value;
        this.element.className = classname;
        this.element.addEventListener('change', (e) => { this.updateValue(e); });
        this.value = value;
    }

    updateValue(e) {
        this.value = e.target.checked;
        this.callback(this);
    }
}
