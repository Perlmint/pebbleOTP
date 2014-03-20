var initialized = false;

Pebble.addEventListener("ready", function() {
	initialized = true;
});

Pebble.addEventListener("showConfiguration", function() {
	Pebble.openURL('');
});