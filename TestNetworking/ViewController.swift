import UIKit

private let UIQueue = DispatchQueue.main

final class ViewController: UIViewController {
    @IBOutlet private var errorLabel: UILabel!
    @IBOutlet private var logLabel: UITextView!
    @IBOutlet private var button: UIButton!

    override func viewDidLoad() {
        super.viewDidLoad()
        self.connect()
    }

    @IBAction private func connect() {
        self.logLabel.text = ""
        self.errorLabel.text = "Connecting"

        DispatchQueue.global(qos: .background).async {
            UIQueue.async { self.button.isHidden = true }

            let error = UnsafeMutablePointer<CChar>.allocate(capacity: 120)
            connectAndRead(error, 120, { received in
                guard let received = received else {
                    return
                }

                UIQueue.async {
                    self.errorLabel.text = "Connected"
                    self.write(line: String(cString: received))

                    let offset = CGPoint(
                        x: 0,
                        y: self.logLabel.contentSize.height - self.logLabel.bounds.size.height
                    )

                    if offset.y > 0 {
                        self.logLabel.setContentOffset(offset, animated: true)
                    }
                }
            })

            let errorAsString = String(cString: error)
            UIQueue.async {
                self.button.isHidden = false
                self.errorLabel.text = errorAsString.isEmpty ? "Disconnected" : errorAsString
                self.logLabel.text = "No data"
            }
        }
    }

    private func write(line: String) {
        let formatter = DateFormatter()
        formatter.dateFormat = "[HH:mm:ss]\t"

        let now = formatter.string(from: Date())
        let attributedLine = NSMutableAttributedString(string: now + line)
        attributedLine.addAttribute(.foregroundColor, value: UIColor.yellow,
                                    range: NSRange(location: 0, length: now.count))
        attributedLine.addAttribute(.foregroundColor, value: UIColor.white,
                                    range: NSRange(location: now.count, length: line.count))

        let previous = NSMutableAttributedString(attributedString: self.logLabel.attributedText)
        previous.append(attributedLine)

        self.logLabel.attributedText = previous
    }
}
