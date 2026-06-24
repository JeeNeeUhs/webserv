document.addEventListener("DOMContentLoaded", () => {
	const sid = document.cookie
		.split("; ")
		.find(c => c.startsWith("sid="))
		?.split("=")[1];

	const phrases = [
		"hello world",
		"wuck febserv",
		"sata andagi",
		"did you try -d flag?"
	]

	document.getElementById("cookie").innerText = sid || "not found";
	document.getElementsByClassName("animText")[0].innerText = phrases[Math.floor(Math.random() * phrases.length)];
});

console.log("zarttiri zort zort cankardesler");