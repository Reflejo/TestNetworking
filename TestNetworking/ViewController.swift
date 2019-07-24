import UIKit

class ViewController: UIViewController {
    @IBOutlet private var errorLabel: UILabel!
    @IBOutlet private var logLabel: UITextView!

    override func viewDidLoad() {
        super.viewDidLoad()

        DispatchQueue.global(qos: .background).async {
            let error = UnsafeMutablePointer<CChar>.allocate(capacity: 120)
            connectAndRead(error, 120, { received in
                guard let received = received else {
                    return
                }

                DispatchQueue.main.async {
                    self.logLabel.insertText(Date().description + "\t" + String(cString: received))

                    let offset = CGPoint(
                        x: 0,
                        y: self.logLabel.contentSize.height - self.logLabel.bounds.size.height
                    )

                    if offset.y > 0 {
                        self.logLabel.setContentOffset(offset, animated: true)
                    }
                }
            })

            DispatchQueue.main.async {
                self.errorLabel.text = String(cString: error)
            }
        }
    }
}
